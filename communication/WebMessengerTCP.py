import json
import socket
import threading
from typing import Callable

from .BaseMessenger import BaseServer


class WebMessengerTCP(threading.Thread, BaseServer):

    def __init__(self, port: int, data_handler: Callable, connect_handler: Callable, unconnect_handler: Callable, logger: Callable = print) -> None:
        threading.Thread.__init__(self)
        BaseServer.__init__(self, data_handler, connect_handler, unconnect_handler, logger)
        self.port = port
        self.clients = {}
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.bind(('0.0.0.0', self.port))

    def run(self) -> None:
        self.server.listen()
        while True:
            client_socket, client_address = self.server.accept()
            threading.Thread(target=self.__client_handler, args=(client_socket, client_address)).start()

    def __client_handler(self, client_socket: socket.socket, client_address: tuple) -> None:
        info = self.connect_handler(client_socket, client_address)
        self.clients[info['machine_id']] = client_socket
        self.logger(f'Client {info["machine_id"]} (host: {client_address}) connected')
        while True:
            try:
                data = client_socket.recv(1024)
                if not data:
                    break
                self.data_handler(data, info['machine_id'])
            except ConnectionResetError:
                self.unconnect_handler(info['machine_id'])
                break
        client_socket.close()
        del self.clients[client_address]

    def send(self, data: dict, machine_id: int | str) -> None:
        data = json.dumps(data).encode()
        self.clients[machine_id].send(data)
