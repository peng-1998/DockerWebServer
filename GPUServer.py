import os

import yaml

from communication import BaseClient, DockerController
import communication

os.getcwd(os.path.dirname(__file__))

if __name__ == '__main__':
    configs = yaml.load(open('config.yaml', 'r', encoding='utf-8'), Loader=yaml.FullLoader)
    Messenger_Class = getattr(communication, configs['Components']['WebMessenger']['Class'])
    messenger: BaseClient = Messenger_Class(**configs['Components']['WebMessenger']['args'], data_handler=data_handler, connect_handler=connect_handler, disconnect_handler=disconnect_handler, logger=g.logger)
    messenger.start()  # start the messenger thread
    messenger.join()
