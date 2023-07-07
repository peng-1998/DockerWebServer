import json
import socket
import threading
import time
from typing import Callable

from .BaseMessenger import BaseServer


class WebMessengerTCP(threading.Thread, BaseServer):
    """
    Creating one thread for listening to the port.
    Creating another thread for checking the heartbeat.
    Once a client is connected, a new thread is created for the client.
    __client_handler: 
        input: client_socket (the object of the connected client's socket), client_address (the tuple address of the client)
        logic: Polling the event loop whether there is connection request from the client, and if there is, receive the data from the client and call the data_handler.
    send:
        input: data (the data to be sent), machine_id (the id of the client)
        logic: Send the data to the client with the given machine_id.
    """
    def __init__(self, port: int, data_handler: Callable, connect_handler: Callable, disconnect_handler: Callable, max_hreatbeat_timeout: float = 10.0) -> None:
        threading.Thread.__init__(self)
        BaseServer.__init__(self, data_handler, connect_handler, disconnect_handler)
        self.port = port
        self.clients = {}
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server.bind(('0.0.0.0', self.port))
        self.daemon = True
        self.max_hreatbeat_timeout = max_hreatbeat_timeout

    def run(self) -> None:
        self.server.listen()
        threading.Thread(target=self.__check_heartbeat).start()
        while True:
            client_socket, client_address = self.server.accept()
            threading.Thread(target=self.__client_handler, args=(client_socket, client_address)).start()

    def __client_handler(self, client_socket: socket.socket, client_address: tuple) -> None:
        content_json = self.__read_msg(client_socket)
        assert content_json['type'] == 'init'
        ip, _ = client_address
        info = self.connect_handler(content_json['data'], ip)
        thread_id = threading.get_ident()
        self.clients[info['machine_id']] = {'socket': client_socket, 'thread_id': thread_id, 'last_heartbeat_time': time.time()}
        while True:
            try:
                content_json = self.__read_msg(client_socket)
                if content_json is None:
                    break
                if content_json['type'] != 'heartbeat':
                    self.data_handler(content_json, info['machine_id'])
                self.clients[info['machine_id']]['last_heartbeat_time'] = time.time()
            except ConnectionResetError:
                break
        client_socket.close()
        self.disconnect_handler(info['machine_id'])
        del self.clients[info['machine_id']]

    def send(self, data: dict, machine_id: int | str) -> None:
        data = json.dumps(data).encode()
        self.clients[machine_id]['socket'].send(data)

    def send_all(self, data: dict) -> None:
        data = json.dumps(data).encode()
        for client in self.clients.values():
            client['socket'].send(data)

    def __check_heartbeat(self):
        while True:
            for key, value in list(self.clients.items()):
                if time.time() - value['last_heartbeat_time'] > self.max_hreatbeat_timeout:
                    self.disconnect_handler(key)
                    tr_id = value['thread_id']
                    value['socket'].close()
                    for th in threading.enumerate():
                        if th.ident == tr_id:
                            th._stop()
                    del self.clients[key]
            time.sleep(1)

    def __read_msg(self, client_socket: socket.socket) -> dict:
        content = b''
        while True:
            data = client_socket.recv(1024)
            if not data:
                return None
            content += data
            try:
                content_json = json.loads(content.decode())
                return content_json
            except json.decoder.JSONDecodeError:
                pass
