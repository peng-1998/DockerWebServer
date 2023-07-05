from collections.abc import Callable, Iterable, Mapping
import websockets
from websockets.server import WebSocketServerProtocol
from websockets.exceptions import ConnectionClosedError
from typing import Any, Callable
import threading
import asyncio
import json
import time
from .BaseMessenger import BaseServer


class WebMassageWS(BaseServer, threading.Thread):
    """
    Creating one thread for listening to the port.
    Using coroutines to check the heartbeat and handle the data.
    Once a client is connected, a new coroutine is created for the client.
    __client_handler:
        input: websocket (the object of the connected client's websocket), path (the path of the client)
        logic: Receive the data from the client and call the data_handler.
    send:
        input: data (the data to be sent), machine_id (the id of the client)
        logic: Send the data to the client with the given machine_id.
    """

    def __init__(
        self,
        port: int,
        data_handler: Callable[..., Any],
        connect_handler: Callable[..., Any],
        disconnect_handler,
        max_hreatbeat_timeout: float = 10.0,
    ) -> None:
        super().__init__(data_handler, connect_handler, disconnect_handler)
        threading.Thread.__init__(self)
        self.port = port
        self.data_handler = data_handler
        self.connected_clients_ws = {}
        self.connected_clients_hearbeat_arrival_time = {}
        self.max_hreatbeat_timeout = max_hreatbeat_timeout

    async def _check_heartbeat(self):
        while True:
            # 对所有已连接的客户端进行心跳检测,心跳检测包括：检查接受数据包hb类型的间隔时间。
            for m_id, ws in self.connected_clients_ws.items():
                if (
                    time.time() - self.connected_clients_hearbeat_arrival_time[m_id]
                    > self.max_hreatbeat_timeout
                ):
                    print("heartbeat timeout")
                    self.disconnect_handler(m_id)
                    ws.close()
                    del self.connected_clients_ws[m_id]
                    del self.connected_clients_hearbeat_arrival_time[m_id]
            await asyncio.sleep(5)

    async def dispatcher(self, data: dict, ws: WebSocketServerProtocol):
        "对数据包进行分发处理,分发为心跳包和数据包,更新心跳包接受时间，对数据包进行处理"
        if data["type"] == "heartbeat":
            self.connected_clients_hearbeat_arrival_time[
                data["data"]["machine_id"]
            ] = time.time()
        else:
            await self.data_handler(data, ws)

    async def _main(self):
        heartbeat_task = self._check_heartbeat()
        server = websockets.serve(self._connector_handler, "localhost", self.port)
        await asyncio.gather(heartbeat_task, server)

    async def _connector_handler(self, websocket, path):
        await self.registe_ws(websocket)
        while True:
            try:
                data = await websocket.recv()
                data = json.loads(data)
                await self.dispatcher(data)
            except ConnectionClosedError as e:
                break

    async def registe_ws(self, websocket: WebSocketServerProtocol) -> None:
        "注册websocket链接信息以及心跳信息,并调用connect_handler"
        data = await websocket.recv()
        data = json.loads(data)
        assert data["type"] == "init"
        ip, port = websocket.remote_address
        self.connected_clients_ws[data["data"]["machine_id"]] = websocket
        self.connected_clients_hearbeat_arrival_time[
            data["data"]["machine_id"]
        ] = time.time()
        await self.connect_handler(data["data"], ip)

    def send(self, data: dict, machine_id: int | str) -> None:
        self.connected_clients_ws["machine_id"].send(data)

    def send_all(self, data: dict) -> None:
        for client in self.connected_clients_ws.values():
            client.send(data)

    def run(self):
        asyncio.run(self._main())
