import threading
import socket


class GPUWebMessengerTCP(threading.Thread):

    def __init__(self, host: str, port: int, data_handler) -> None:
        super().__init__()
        self.port = port
        self.host = host
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.connect((self.host, self.port))
        self.data_handler = data_handler

    def run(self) -> None:
        while True:
            data = self.server.recv(1024)
            if not data:
                break
            self.data_handler(data)

    def send(self, data: bytes) -> None:
        self.server.send(data)
