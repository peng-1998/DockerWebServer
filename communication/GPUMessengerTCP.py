import json
import socket
import threading
import time
from typing import Callable
from .BaseMessenger import BaseClient


class GPUWebMessengerTCP(threading.Thread, BaseClient):

    def __init__(self, host: str, port: int, data_handler: Callable, connect_handler: Callable = None, init_reconnect_interval: int = 1, max_reconnect_interval: int = 30, logger=print) -> None:
        threading.Thread.__init__(self)
        BaseClient.__init__(self, data_handler, logger)
        self.port = port
        self.host = host
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.connected = False
        self.data_handler = data_handler
        self.connect_handler = connect_handler
        self.init_reconnect_interval = init_reconnect_interval
        self.max_reconnect_interval = max_reconnect_interval
        self.logger = logger

    def run(self) -> None:
        reconnect_interval = self.init_reconnect_interval
        while True:
            try:
                self.server.connect((self.host, self.port))
                self.connected = True
                self.logger("Connected to server")
                if self.connect_handler:
                    self.connect_handler()
                while True:
                    data = self.server.recv(1024)
                    if not data:
                        break
                    self.data_handler(data)
            except socket.error:
                self.connected = False
                self.logger(f'Connection failed. Reconnecting in {reconnect_interval}s...')
                time.sleep(reconnect_interval)
                reconnect_interval = min(self.max_reconnect_interval, 2 * reconnect_interval)  # 指数退避
            else:
                reconnect_interval = self.init_reconnect_interval  # 连接成功,重置重连间隔

    def send(self, data: dict) -> None:
        data = json.dumps(data).encode()
        self.server.send(data)
