from typing import Any, Callable
from utils.AsyncStructure import AsyncDict
from quart import Websocket
import asyncio
from .BaseMessenger import BaseServer


class ServerManager(BaseServer, AsyncDict):

    def __init__(self, data_handler: Callable[..., Any], connect_handler: Callable[..., Any], disconnect_handler: Callable[..., Any], max_hreatbeat_timeout: float = 10.0) -> None:
        super().__init__(data_handler, connect_handler, disconnect_handler)
        asyncio.create_task(self.__check_heartbeat())
        self.max_hreatbeat_timeout = max_hreatbeat_timeout

    async def __setitem__(self, key, value):
        async with self._lock:
            loop = asyncio.get_running_loop()
            last_heartbeat_time = loop.time()
            self._data[key] = {'ws': value, 'last_heartbeat_time': last_heartbeat_time}
            asyncio.create_task(self.__receiver(key, value))

    async def __receiver(self, machine_id: int | str, websocket: Websocket):
        loop = asyncio.get_running_loop()
        try:
            while True:
                data = await websocket.receive_json()
                if data['type'] == 'heartbeat':
                    async with self._lock:
                        self._data[machine_id]['last_heartbeat_time'] = loop.time()
                else:
                    await self.data_handler(data, machine_id)
        except asyncio.CancelledError:
            self.disconnect_handler(machine_id)
            async with self._lock:
                del self._data[machine_id]


    async def send(self, data: dict, machine_id: int | str) -> None:
        await self._data[machine_id]['ws'].send_json(data)

    async def send_all(self, data: dict) -> None:
        for client in self._data.values():
            await client['ws'].send_json(data)

    async def __check_heartbeat(self):
        loop = asyncio.get_running_loop()
        while True:
            for key, value in list(self.clients.items()):
                if loop.time() - value['last_heartbeat_time'] > self.max_hreatbeat_timeout:
                    self.disconnect_handler(key)
                    value['ws'].close()
            await asyncio.sleep(1)
