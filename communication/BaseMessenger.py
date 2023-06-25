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

    def __init__(self, data_handler: Callable, connect_handler: Callable, logger: Callable = print) -> None:
        super().__init__()
        self.data_handler = data_handler
        self.connect_handler = connect_handler
        self.logger = logger

    @abstractmethod
    def send(self, data: dict, machine_id: int | str) -> None:
        pass


class BaseClient(ABC):

    def __init__(self, data_handler: Callable, connect_handler: Callable, logger: Callable = print) -> None:
        super().__init__()
        self.data_handler = data_handler
        self.connect_handler = connect_handler
        self.logger = logger

    @abstractmethod
    def send(self, data: dict) -> None:
        pass
