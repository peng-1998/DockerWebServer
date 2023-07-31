import json
import threading
from typing import Callable
import asyncio
from asyncio.streams import StreamReader, StreamWriter

from .BaseMessenger import BaseServer


class WebMessengerTCPAsync(threading.Thread, BaseServer):

    def __init__(self, port: int, data_handler: Callable, connect_handler: Callable, disconnect_handler: Callable, max_hreatbeat_timeout: float = 10.0) -> None:
        threading.Thread.__init__(self)
        BaseServer.__init__(self, data_handler, connect_handler, disconnect_handler)
        self.port = port
        self.clients = {}
        self.daemon = True
        self.max_hreatbeat_timeout = max_hreatbeat_timeout

    def run(self) -> None:
        asyncio.run(self.__run_server('0.0.0.0', self.port))

    async def __run_server(self, host, port):
        loop = asyncio.get_running_loop()
        loop.create_task(self.__check_heartbeat())
        server = await asyncio.start_server(self.__client_handler, host, port)
        print(f"Server started on {host}:{port}")
        async with server:
            await server.serve_forever()

    async def __client_handler(self, reader: StreamReader, writer: StreamWriter):
        content_json = await self.__read_msg(reader)
        assert content_json['type'] == 'init'
        ip, _ = writer.get_extra_info('peername')
        info = self.connect_handler(content_json['data'], ip)
        task = asyncio.current_task()
        self.clients[info['machine_id']] = {'reader': reader, 'writer': writer, 'task': task, 'last_heartbeat_time': loop.time()}
        content = b''
        while True:
            try:
                content_json = await self.__read_msg(reader)
                if content_json is None:
                    break
                if content_json['type'] != 'heartbeat':
                    self.data_handler(content_json, info['machine_id'])
                self.clients[info['machine_id']]['last_heartbeat_time'] = loop.time()
            except ConnectionResetError:
                break
        reader.feed_eof()
        writer.close()
        self.disconnect_handler(info['machine_id'])
        del self.clients[info['machine_id']]

    def send(self, data: dict, machine_id: int | str) -> None:
        self.clients[machine_id]['writer'].write(json.dumps(data).encode())

    def send_all(self, data: dict) -> None:
        data = json.dumps(data).encode()
        for value in self.clients.values():
            value['writer'].write(data)

    async def __check_heartbeat(self):
        loop = asyncio.get_running_loop()
        while True:
            for key, value in list(self.clients.items()):
                if loop.time() - value['last_heartbeat_time'] > self.max_hreatbeat_timeout:
                    self.disconnect_handler(key)
                    value['writer'].close()
                    value['reader'].feed_eof()
                    value['task'].cancel()
                    del self.clients[key]
            await asyncio.sleep(1)

    async def __read_msg(self, reader: StreamReader) -> dict | None:
        content = b''
        while True:
            data = await reader.read(1024)
            if not data:
                return None
            content += data
            try:
                content_json = json.loads(content.decode())
                return content_json
            except json.decoder.JSONDecodeError:
                pass