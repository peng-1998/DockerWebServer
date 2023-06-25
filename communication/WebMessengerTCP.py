import threading
import socket

class WebMessengerTCP(threading.Thread):

    def __init__(self, port: int,data_handler,logger=print) -> None:
        super().__init__()
        self.port = port
        self.clients = {}
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.bind(('0.0.0.0', self.port))
        self.data_handler = data_handler
        self.logger = logger

    def run(self) -> None:
        self.server.listen()
        while True:
            client_socket, client_address = self.server.accept()
            self.clients[client_address] = client_socket
            self.logger(f'Client {client_address} connected')
            threading.Thread(target=self.__client_handler, args=(client_socket, client_address)).start()

    def __client_handler(self, client_socket: socket.socket, client_address: tuple) -> None:
        while True:
            try:
                data = client_socket.recv(1024)
                if not data:
                    break
                self.data_handler(data, client_socket, client_address)
            except ConnectionResetError:
                break
        client_socket.close()
        del self.clients[client_address]

    def send(self, data: bytes, client_address: tuple) -> None:
        self.clients[client_address].send(data)



