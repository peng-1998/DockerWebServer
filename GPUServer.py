import os
import threading

from docker.errors import APIError as DKAPIError
import yaml
from docker.models.containers import Container, Image

import communication
from communication import NVIDIAGPU, BaseClient, DockerController
from communication.utils import get_cpu_info, get_disk_info, get_memory_info

if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))

    def pull_image(image: str, tag: str):
        for _ in range(3):
            res = docker_controller.pull_image(image, tag)
            if isinstance(res, Image):
                messenger.send({'type': 'image', 'data': {'image': image, 'opt': 'pull', 'status': 'success'}})
                break

        else:
            messenger.send({'type': 'image', 'data': {'image': image, 'opt': 'pull', 'status': 'failed', 'error': str(res)}})

    def remove_image(image: str, tag: str):
        res = docker_controller.remove_image(image, tag)
        if isinstance(res, list):
            messenger.send({'type': 'image', 'data': {'image': image, 'opt': 'remove', 'status': 'success'}})
        else:
            messenger.send({'type': 'image', 'data': {'image': image, 'opt': 'remove', 'status': 'failed', 'error': str(res)}})

    image_funcs = {'pull': pull_image, 'remove': remove_image}

    def data_handler_image(data: dict):
        threading.Thread(target=image_funcs[data['opt']], args=(data['image'], data['tag'])).start()

    def data_handler_container(data: dict):
        if data['opt'] == 'create':
            res = docker_controller.create_container(**data['create_args'])
            if isinstance(res, Container):
                messenger.send({'type': 'container', 'data': {'opt': 'create', 'user_id': data['user_id'], 'status': 'success', 'original_data': data['create_args']}})
            else:
                messenger.send({'type': 'container', 'data': {'opt': 'create', 'user_id': data['user_id'], 'status': 'failed', 'error': str(res), 'original_data': data['create_args']}})
        else:
            container = docker_controller.get_container(data['container_name'])
            if not container:
                messenger.send({'type': 'container', 'data': {'opt': data['opt'], 'user_id': data['user_id'], 'status': 'failed', 'error': 'container not found', 'container_name': data['container_name']}})
                return
            try:
                if (data['opt'] == 'start'):
                    container.start()
                elif (data['opt'] == 'stop'):
                    container.stop()
                elif (data['opt'] == 'remove'):
                    container.remove(force=True)
                elif (data['opt'] == 'restart'):
                    container.restart()
                messenger.send({'type': 'container', 'data': {'opt': data['opt'], 'user_id': data['user_id'], 'status': 'success', 'container_name': data['container_name']}})
            except DKAPIError as e:
                messenger.send({'type': 'container', 'data': {'opt': data['opt'], 'user_id': data['user_id'], 'status': 'failed', 'error': str(e), 'container_name': data['container_name']}})

    data_handler_funcs = {
        'image': data_handler_image,
        'container': data_handler_container,
    }

    def data_handler(data: dict):
        data_handler_funcs[data['type']](data['data'])

    def connect_handler():
        init_data = {'type': 'init', 'data': {'machine_id': configs['MachineID'], 'gpus': nvidia_gpu.gpu_info, 'cpu': get_cpu_info(), 'memory': get_memory_info(), 'disk': get_disk_info(), 'url': configs['URL']}}
        messenger.send(init_data)

    configs = yaml.load(open('GPUServerConfig.yaml', 'r', encoding='utf-8'), Loader=yaml.FullLoader)
    docker_controller = DockerController()
    nvidia_gpu = NVIDIAGPU()

    Messenger_Class = getattr(communication, configs['Components']['WebMessenger']['Class'])
    messenger: BaseClient = Messenger_Class(**configs['Components']['WebMessenger']['args'], data_handler=data_handler, connect_handler=connect_handler, logger=print)
    messenger.start()  # start the messenger thread
    messenger.join()
