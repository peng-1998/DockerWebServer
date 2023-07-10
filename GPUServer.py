import asyncio
import os
import time
import threading

from docker.errors import APIError as DKAPIError
import yaml
from docker.models.containers import Container, Image

import communication
from communication import NVIDIAGPU, BaseClient, DockerController
from communication.utils import get_cpu_info, get_disk_info, get_memory_info
from utils.AsyncStructure import AsyncDict

if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))

    async def pull_image(image: str, uuid: str):
        res_msg = {
            'type': 'image',
            'data': {
                'image': image,
                'opt': 'pull',
                'uuid': uuid,
            }
        }
        if docker_controller.get_image(image) is None:
            res_msg['data']['status'] = 'exists'
            await messenger.send(res_msg)
            return
        loop = asyncio.get_event_loop()
        for _ in range(3):
            res = await loop.run_in_executor(None, docker_controller.pull_image, image)
            if isinstance(res, Image):
                res_msg['data']['status'] = 'success'
                await messenger.send(res_msg)
                break
        else:
            res_msg['data']['status'] = 'failed'
            res_msg['data']['error'] = str(res)
            await messenger.send(res_msg)

    async def remove_image(image: str, uuid):
        res_msg = {
            'type': 'image',
            'data': {
                'image': image,
                'opt': 'remove',
                'uuid': uuid,
            }
        }
        if docker_controller.get_image(image) is None:
            res_msg['data']['status'] = 'not_exists'
            await messenger.send(res_msg)
            return
        loop = asyncio.get_event_loop()
        res = await loop.run_in_executor(None, docker_controller.remove_image, image)
        if isinstance(res, list):
            res_msg['data']['status'] = 'success'
            await messenger.send(res_msg)
        else:
            res_msg['data']['status'] = 'failed'
            res_msg['data']['error'] = str(res)
            await messenger.send(res_msg)

    image_funcs = {'pull': pull_image, 'remove': remove_image}

    async def data_handler_image(data: dict):
        await image_funcs[data['opt']](data['image'], data['uuid'] if 'uuid' in data else None)

    async def data_handler_container(data: dict):
        loop = asyncio.get_event_loop()
        res_msg = {
            'type': 'container',
            'data': {
                'opt': data['opt'],
                'user_id': data['user_id'],
                'container_name': data['container_name'],
                'uuid': data['uuid'],
            }
        }
        if data['opt'] == 'create':
            image_name = data['create_args']['image']
            image = docker_controller.get_image(image_name)
            if not image:
                msg = res_msg.copy()
                msg['data']['status'] = 'pulling'
                await messenger.send(msg)
                await loop.run_in_executor(None, docker_controller.pull_image, image_name)
            res = await loop.run_in_executor(None, docker_controller.create_container, **data['create_args'])
            res_msg['data']['original_data'] = data['create_args']
            if isinstance(res, Container):
                res_msg['data']['status'] = 'success'
            else:
                res_msg['data']['status'] = 'failed'
                res_msg['data']['error'] = str(res)
            await messenger.send(res_msg)
            return
        container = docker_controller.get_container(data['container_name'])
        if not container:
            res_msg['data']['status'] = 'failed'
            res_msg['data']['error'] = 'container not found'
            await messenger.send(res_msg)
            return

        if data['opt'] == 'commit':
            res = await loop.run_in_executor(None, container.commit, tag=data['account'] + '/' + data['image_name'] + ':' + 'latest')
            res = await loop.run_in_executor(None, docker_controller.push_image, res.tags)
            if res is None:
                res_msg['data']['status'] = 'success'
            else:
                res_msg['data']['status'] = 'failed'
                res_msg['data']['error'] = str(res)
            await messenger.send(res_msg)
            return

        try:
            if (data['opt'] == 'start'):
                await loop.run_in_executor(None, container.start)
            elif (data['opt'] == 'stop'):
                await loop.run_in_executor(None, container.stop)
            elif (data['opt'] == 'remove'):
                await loop.run_in_executor(None, container.remove, force=True)
            elif (data['opt'] == 'restart'):
                await loop.run_in_executor(None, container.restart)
            res_msg['data']['status'] = 'success'
            await messenger.send(res_msg)
        except DKAPIError as e:
            res_msg['data']['status'] = 'failed'
            res_msg['data']['error'] = str(e)
            await messenger.send(res_msg)

    async def data_handler_task(data: dict):
        gpus = data['gpus']
        res_msg = {'type': 'task', 'data': {'opt': data['opt'], 'user_id': data['user_id'], 'task_id': data['task_id'], 'container_name': data['container_name']}}
        if data['container_name'] == '':
            loop = asyncio.get_event_loop()
            container = docker_controller.get_container(data['container_name'])
            if not container:
                res_msg['data']['status'] = 'failed'
                res_msg['data']['error'] = 'container not found'
                await messenger.send(res_msg)
                return
        if data['opt'] == 'finish':
            await loop.run_in_executor(None, container.restart)
            await asyncio.sleep(0.5)
            res_msg['data']['status'] = 'success'
            await messenger.send(res_msg)
            for g in gpus:
                await gpus_dict.__setitem__(g, '')
            return
        # start task
        if data['container_name'] == '':
            if container.status != 'running':
                await loop.run_in_executor(None, container.start)
                await messenger.send({'type': 'container', 'data': {'opt': 'start', 'user_id': data['user_id'], 'status': 'success', 'container_name': data['container_name']}})
            try:
                if data['cmd'] != '':
                    await loop.run_in_executor(None, container.exec_run, data['cmd'])
                res_msg['data']['status'] = 'success'
                for g in gpus:
                    await gpus_dict.__setitem__(g, '')
            except DKAPIError as e:
                res_msg['data']['status'] = 'failed'
                res_msg['data']['error'] = str(e)
            await messenger.send(res_msg)

    data_handler_funcs = {'image': data_handler_image, 'container': data_handler_container, 'task': data_handler_task}

    async def data_handler(data: dict):
        await data_handler_funcs[data['type']](data['data'])

    async def connect_handler():
        # send init data
        containers: list[Container] = docker_controller.containers
        container_data = [{'name': container.name, 'running': container.status == 'running'} for container in containers]

        init_data = {
            'type': 'init',
            'data': {
                'machine_id': configs['MachineID'],
                'gpus': nvidia_gpu.gpu_info,
                'cpu': get_cpu_info(),
                'memory': get_memory_info(),
                'disk': get_disk_info(),
                'url': configs['URL'],
                'containers': container_data,
            }
        }
        await messenger.send(init_data)

    async def send_gpus():
        while True:
            await asyncio.sleep(2)
            await messenger.send({'type': 'gpus', 'data': nvidia_gpu.gpu_info})

    configs = yaml.load(open('GPUServerConfig.yaml', 'r', encoding='utf-8'), Loader=yaml.FullLoader)
    docker_controller = DockerController()
    nvidia_gpu = NVIDIAGPU()

    Messenger_Class = getattr(communication, configs['Components']['WebMessenger']['Class'])
    messenger: BaseClient = Messenger_Class(**configs['Components']['WebMessenger']['args'], machine_id=configs['MachineID'], data_handler=data_handler, connect_handler=connect_handler)

    gpus_dict = AsyncDict()

    def main():
        loop = asyncio.get_event_loop()
        loop.create_task(messenger.start())
        loop.create_task(send_gpus())
        loop.create_task(kill_process())
        loop.run_forever()

    async def kill_process():
        gpus = nvidia_gpu.gpucount
        for i in range(gpus):
            await gpus_dict.__setitem__(i, '')
        while True:
            for i in range(gpus):
                await asyncio.sleep(2)
                account = await gpus_dict.__getitem__(i)
                if account != '':
                    containers = [container for container in docker_controller.containers if account in container.name]
                    pids = []
                    for container in containers:
                        pids+=[int(pid) for pid in container.top()['Processes']]
                    gpu_pids = nvidia_gpu.running_processes(i)
                    for gpu_pid in gpu_pids:
                        if gpu_pid not in pids:
                            # os.system('kill -9 ' + str(gpu_pid))
                            print('kill -9 ' + str(gpu_pid))

    asyncio.run(main())