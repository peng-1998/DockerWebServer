import json
import socket
import threading
import time
from typing import Callable
from .BaseMessenger import BaseClient


class GPUWebMessengerTCP(threading.Thread, BaseClient):

    def __init__(self, host: str, port: int, data_handler: Callable, connect_handler: Callable = None, init_reconnect_interval: int = 1, max_reconnect_interval: int = 30, max_hreatbeat_timeout: float = 1.0, logger: Callable = print) -> None:
        """ The TCP client for GPU client

        Args:
            host (str): the ip or url of the web server
            port (int): the web server's port
            data_handler (Callable): an function to handle the data from server, it gets a dict as parameter
            connect_handler (Callable, optional): an function to will be called when the client connected to server. Defaults to None.
            init_reconnect_interval (int, optional): the initial reconnect interval. Defaults to 1.
            max_reconnect_interval (int, optional): the max reconnect interval. Defaults to 30.
            max_hreatbeat_timeout (float, optional): the max time interval between two heartbeats. Defaults to 5.0.
            logger (Callable, optional): the logger function. Defaults to print.
        """
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
        self.last_heartbeat_time = time.time()
        self.max_hreatbeat_timeout = max_hreatbeat_timeout

    def run(self) -> None:
        reconnect_interval = self.init_reconnect_interval
        threading.Thread(target=self.__send_hreatbeat).start() # 启动心跳线程
        while True:
            try:
                self.server.connect((self.host, self.port))
                self.connected = True
                self.logger("Connected to server")
                if self.connect_handler:
                    self.connect_handler()
                while True:
                    data = self.server.recv(1024)
                    if not data: # 说明连接断开
                        self.connected = False
                        self.logger("server disconnected")
                        break
                    data = json.loads(data.decode())
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
        self.last_heartbeat_time = time.time()

    def __send_hreatbeat(self):
        while True:
            if self.connected:
                if time.time() - self.last_heartbeat_time > self.max_hreatbeat_timeout:
                    self.send({"type": "heartbeat", "data": {}})
