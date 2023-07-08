from abc import ABC, abstractmethod
from typing import Callable
'''
message format:
{
    "type": "init" | "query-info" | "update-image" | "container-operation" | "heartbeat"  ,
    "data": dict,
}
'''


class BaseServer(ABC):

    def __init__(self, data_handler: Callable, connect_handler: Callable, disconnect_handler: Callable) -> None:
        super().__init__()
        self.data_handler = data_handler
        self.connect_handler = connect_handler
        self.disconnect_handler = disconnect_handler

    @abstractmethod
    def send(self, data: dict, machine_id: int | str) -> None:
        pass

    @abstractmethod
    def send_all(self, data: dict) -> None:
        pass


class BaseClient(ABC):

    def __init__(self, data_handler: Callable, connect_handler: Callable) -> None:
        super().__init__()
        self.data_handler = data_handler
        self.connect_handler = connect_handler

    @abstractmethod
    def send(self, data: dict) -> None:
        pass