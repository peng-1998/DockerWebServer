from typing import Any, Callable
from utils.AsyncStructure import AsyncDict
from quart import Websocket
import asyncio
from .BaseMessenger import BaseServer


class ServerManager(BaseServer, AsyncDict):

    def __init__(self, data_handler: Callable[..., Any], connect_handler: Callable[..., Any], disconnect_handler: Callable[..., Any], max_hreatbeat_timeout: float = 3.) -> None:
        super().__init__(data_handler, connect_handler, disconnect_handler)
        asyncio.get_event_loop().create_task(self.__check_heartbeat())
        self.max_hreatbeat_timeout = max_hreatbeat_timeout

    async def __setitem__(self, key, value):
        loop = asyncio.get_event_loop()
        last_heartbeat_time = loop.time()
        task = loop.create_task(self.__receiver(key, value))
        async with self._lock:
            self._data[key] = {'task': task, 'ws': value, 'last_heartbeat_time': last_heartbeat_time}
        await task

    async def __receiver(self, machine_id: int | str, websocket: Websocket):
        loop = asyncio.get_running_loop()
        try:
            first_data = await websocket.receive_json()
            await self.connect_handler(first_data['data'])
            while True:
                data = await websocket.receive_json()
                if data['type'] == 'heartbeat':
                    async with self._lock:
                        self._data[machine_id]['last_heartbeat_time'] = loop.time()
                else:
                    await self.data_handler(data, machine_id)
        except asyncio.CancelledError:
            await self.disconnect_handler(machine_id)
            async with self._lock:
                self._data.__delitem__(machine_id)

    async def send(self, data: dict, machine_id: int | str) -> None:
        await self._data[machine_id]['ws'].send_json(data)

    async def send_all(self, data: dict) -> None:
        for client in self._data.values():
            await client['ws'].send_json(data)

    async def __check_heartbeat(self):
        loop = asyncio.get_running_loop()
        while True:
            async with self._lock:
                for key, value in list(self._data.items()):
                    if loop.time() - value['last_heartbeat_time'] > self.max_hreatbeat_timeout:
                        await self.disconnect_handler(key)
                        value['task'].cancel()
            await asyncio.sleep(1)
