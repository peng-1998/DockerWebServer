from collections.abc import Callable, Iterable, Mapping
import websockets
from typing import Any, Callable
import threading
import asyncio
import json
from .BaseMessenger import BaseServer


class WebMassageWS(BaseServer,threading.Thread):
    def __init__(self, port: int, data_handler: Callable[..., Any], connect_handler: Callable[..., Any], disconnect_handler, logger: Callable[..., Any] = ...) -> None:
        super().__init__(data_handler, connect_handler, disconnect_handler, logger)
        threading.Thread.__init__(self)
        self.port = port
        self.data_handler = data_handler
        self.clients_ws = {}

    async def _data_handler(self, data_handler: Callable, data) -> None:
        self.logger(f'Client {info["machine_id"]} (host: {client_address}) connected')
        await data_handler(data)

    async def _connect_handler(self, websocket: websockets.WebSocketServerProtocol, path: str) -> None:
        data = 
        self.clients_ws[data['machine_id']] = websocket
        while True:
            try:
                data = await websocket.recv()
                data = json.loads(data.decode())
                await self._data_handler(self.data_handler, data)
            except websockets.exceptions.ConnectionClosedError:
                break
        
    async def run(self):
        server = await websockets.serve(self._connect_handler, "localhost", self.port)
        asyncio.run(server)

    def send(self, data: dict, machine_id: int | str) -> None:
        self.clients_ws['machine_id'].send(data)
    
    def send_all(self, data: dict) -> None:
        for client in self.clients_ws.values():
            client.send(data)



        
        
