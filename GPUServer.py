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

if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))

    async def pull_image(image: str, tag: str):
        for _ in range(3):
            res = await loop.run_in_executor(None, docker_controller.pull_image, image, tag)
            if isinstance(res, Image):
                await messenger.send({'type': 'image', 'data': {'image': image, 'opt': 'pull', 'status': 'success'}})
                break
        else:
            await messenger.send({'type': 'image', 'data': {'image': image, 'opt': 'pull', 'status': 'failed', 'error': str(res)}})

    async def remove_image(image: str, tag: str):
        res = await loop.run_in_executor(None, docker_controller.remove_image, image, tag)
        if isinstance(res, list):
            await messenger.send({'type': 'image', 'data': {'image': image, 'opt': 'remove', 'status': 'success'}})
        else:
            await messenger.send({'type': 'image', 'data': {'image': image, 'opt': 'remove', 'status': 'failed', 'error': str(res)}})

    image_funcs = {'pull': pull_image, 'remove': remove_image}

    async def data_handler_image(data: dict):
        await image_funcs[data['opt']](data['image'], data['tag'])

    async def data_handler_container(data: dict):
        if data['opt'] == 'create':
            image_name = data['create_args']['image']
            image = docker_controller.get_image(image_name)
            if not image:
                await loop.run_in_executor(None, docker_controller.pull_image, image_name)
            res = await loop.run_in_executor(None, docker_controller.create_container, **data['create_args'])
            if isinstance(res, Container):
                await messenger.send({'type': 'container', 'data': {'opt': 'create', 'user_id': data['user_id'], 'status': 'success', 'original_data': data['create_args']}})
            else:
                await messenger.send({'type': 'container', 'data': {'opt': 'create', 'user_id': data['user_id'], 'status': 'failed', 'error': str(res), 'original_data': data['create_args']}})
            return
        container = docker_controller.get_container(data['container_name'])
        if not container:
            await messenger.send({'type': 'container', 'data': {'opt': data['opt'], 'user_id': data['user_id'], 'status': 'failed', 'error': 'container not found', 'container_name': data['container_name']}})
            return

        if data['opt'] == 'commit':
            try:
                res = await loop.run_in_executor(None, container.commit, tag=data['account'] + '/' + data['image_name'] + ':' + 'latest')
                await loop.run_in_executor(None, docker_controller.push_image, res.tags)
                await messenger.send({'type': 'container', 'data': {'opt': 'commit', 'user_id': data['user_id'], 'status': 'success', 'original_data': data['create_args']}})
            except DKAPIError as e:
                await messenger.send({'type': 'container', 'data': {'opt': 'commit', 'user_id': data['user_id'], 'status': 'failed', 'error': str(e), 'original_data': data['create_args']}})
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
            await messenger.send({'type': 'container', 'data': {'opt': data['opt'], 'user_id': data['user_id'], 'status': 'success', 'container_name': data['container_name']}})
        except DKAPIError as e:
            await messenger.send({'type': 'container', 'data': {'opt': data['opt'], 'user_id': data['user_id'], 'status': 'failed', 'error': str(e), 'container_name': data['container_name']}})

    async def data_handler_task(data: dict):
        container = docker_controller.get_container(data['container_name'])
        if not container:
            await messenger.send({'type': 'task', 'data': {'status': 'failed', 'task_id': data['task_id'], 'user_id': data['user_id'], 'error': 'container not found', 'container_name': data['container_name']}})
            return
        if data['opt'] == 'finish':
            await loop.run_in_executor(None, container.restart)
            await messenger.send({'type': 'container', 'data': {'opt': 'restart', 'user_id': data['user_id'], 'status': 'success', 'container_name': data['container_name']}})
            await asyncio.sleep(0.5)
            await messenger.send({'type': 'task', 'data': {'status': 'success', 'task_id': data['task_id'], 'user_id': data['user_id'], 'container_name': data['container_name']}})
            return
        if container.status != 'running':
            await loop.run_in_executor(None, container.start)
            await messenger.send({'type': 'container', 'data': {'opt': 'start', 'user_id': data['user_id'], 'status': 'success', 'container_name': data['container_name']}})

        try:
            if data['cmd'] != '':
                await loop.run_in_executor(None, container.exec_run, data['cmd'])
            await messenger.send({'type': 'task', 'data': {'opt': 'run', 'status': 'success', 'task_id': data['task_id'], 'user_id': data['user_id'], 'container_name': data['container_name']}})
        except DKAPIError as e:
            await messenger.send({'type': 'task', 'data': {'opt': 'run', 'status': 'failed', 'task_id': data['task_id'], 'error': str(e), 'user_id': data['user_id'], 'container_name': data['container_name']}})

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

    loop = asyncio.get_event_loop()
    configs = yaml.load(open('GPUServerConfig.yaml', 'r', encoding='utf-8'), Loader=yaml.FullLoader)
    docker_controller = DockerController()
    nvidia_gpu = NVIDIAGPU()

    Messenger_Class = getattr(communication, configs['Components']['WebMessenger']['Class'])
    messenger: BaseClient = Messenger_Class(**configs['Components']['WebMessenger']['args'], machine_id=configs['MachineID'], data_handler=data_handler, connect_handler=connect_handler)
    messenger.start()