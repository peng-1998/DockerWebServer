import json
from typing import Callable
from .BaseMessenger import BaseClient
import websocket
import asyncio


class GPUMessengerWS(BaseClient):

    def __init__(self, host: str, port: int, path: str, machine_id: int | str, data_handler: Callable, connect_handler: Callable = None, init_reconnect_interval: int = 1, max_reconnect_interval: int = 30, max_hreatbeat_timeout: float = 1.0) -> None:
        BaseClient.__init__(self, data_handler, connect_handler)
        self.port = port
        self.host = host
        self.machine_id = machine_id
        self.daemon = True
        self.connected = False
        self.init_reconnect_interval = init_reconnect_interval
        self.max_reconnect_interval = max_reconnect_interval
        self.max_hreatbeat_timeout = max_hreatbeat_timeout
        self.path = path
        self._lock = asyncio.Lock()
        
    async def start(self) -> None:
        reconnect_interval = self.init_reconnect_interval
        loop = asyncio.get_running_loop()
        self.last_heartbeat_time = loop.time()
        loop.create_task(self.__send_hreatbeat())
        while True:
            try:
                self.websocket = websocket.WebSocket()
                self.websocket.connect(f'ws://{self.host}:{self.port}/{self.path}', header={'machine_id': self.machine_id})
                await self.connect_handler()
                reconnect_interval = self.init_reconnect_interval
                self.connected = True
                while True:
                    data = await loop.run_in_executor(None, self.__load_data)
                    await self.data_handler(data)
            except:
                self.connected = False
                reconnect_interval = min(self.max_reconnect_interval, reconnect_interval := 2 * reconnect_interval)
                await asyncio.sleep(reconnect_interval)

    def __load_data(self) -> dict:
        data = b''
        while True:
            data += self.websocket.recv()
            try:
                data = json.loads(data)
                return data
            except json.JSONDecodeError:
                continue

    async def send(self, data: dict) -> None:
        data = json.dumps(data)
        try:
            self.websocket.send(data)
            async with self._lock:
                self.last_heartbeat_time = asyncio.get_running_loop().time()
        except:
            pass

    async def __send_hreatbeat(self):
        loop = asyncio.get_running_loop()
        while True:
            if self.connected:
                async with self._lock:
                    if loop.time() - self.last_heartbeat_time > self.max_hreatbeat_timeout:
                        self.websocket.send(json.dumps({'type': 'heartbeat', 'data': {}}))
            await asyncio.sleep(1)
