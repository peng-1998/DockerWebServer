import os

import yaml

import communication
from communication import BaseClient, DockerController, NVIDIAGPU
from communication.utils import get_cpu_info, get_memory_info, get_disk_info

if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))

    def data_handler(data: dict):
        print(data)

    def connect_handler(data: dict):
        init_data = {'type': 'init', 'data': {'machine_id': configs['MachineID'], 'gpus': nvidia_gpu.gpu_info, 'cpu': get_cpu_info(), 'memory': get_memory_info(), 'disk': get_disk_info(), 'url': configs['URL']}}
        print(init_data)

    def disconnect_handler(data: dict):
        print(data)

    configs = yaml.load(open('GPUServerConfig.yaml', 'r', encoding='utf-8'), Loader=yaml.FullLoader)
    # docker = DockerController()
    nvidia_gpu = NVIDIAGPU()

    # Messenger_Class = getattr(communication, configs['Components']['WebMessenger']['Class'])
    # messenger: BaseClient = Messenger_Class(**configs['Components']['WebMessenger']['args'], data_handler=data_handler, connect_handler=connect_handler, disconnect_handler=disconnect_handler, logger=print)
    # messenger.start()  # start the messenger thread
    # messenger.join()
    connect_handler(None)