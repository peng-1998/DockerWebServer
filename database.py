import json
from typing import Any
import os


class DataBase_NamedTuple:

    def __init__(self, save_path: str, datatype=None) -> None:
        self.save_path = save_path
        self.datatype = datatype
        if os.path.exists(save_path):
            if datatype is not None:
                self.datas = [self.datatype(*_) for _ in json.load(open(save_path, 'r'))]
        else:
            self.datas = []

    def updateStorage(self) -> None:
        json.dump(self.datas, open(self.save_path, 'w'))

    def query(self, key: str, value: Any, limit: int = None) -> Any:
        r = []
        if limit is None:
            limit = len(self.datas) + 1
        for i in range(len(self.datas) - 1, -1, -1):
            if eval(f'self.datas[i].{key}') == value:
                r.append(self.datas[i])
            if len(r) == limit:
                break
        return r

    def add(self, value: Any) -> None:
        self.datas.append(value)
        self.updateStorage()
class DataBase_TypedDict:

    def __init__(self, save_path: str) -> None:
        self.save_path = save_path
        if os.path.exists(save_path):
            self.datas = json.load(open(save_path, 'r'))
        else:
            self.datas = []

    def updateStorage(self) -> None:
        json.dump(self.datas, open(self.save_path, 'w'))

    def query(self, key: str, value: Any, limit: int = None) -> Any:
        r = []
        if limit is None:
            limit = len(self.datas) + 1
        for i in range(len(self.datas) - 1, -1, -1):
            if self.datas[i][key] == value:
                r.append(self.datas[i])
            if len(r) == limit:
                break
        return r

    def add(self, value: Any) -> None:
        self.datas.append(value)
        self.updateStorage()
