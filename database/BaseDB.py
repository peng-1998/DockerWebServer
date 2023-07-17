from abc import ABC, abstractmethod

"""
Table User {
    id int [pk, increment]
    account varchar
    nickname varchar optional
    password varchar
    email varchar optional
    phone varchar optional
    containers list [ref: > Container.id]
}
Table Image {
    id int [pk, increment]
    showname varchar
    imagename varchar
    creat_args dict
    description dict
}
Table Container {
    id int [pk, increment]
    showname varchar
    containername varchar
    machineid int
    portlist
    image int [ref: > Image.id]
    running bool
}
Table Machine {
    id int [pk, increment]
    ip varchar
    Gpus list
    disk dict
    memory dict
}
"""


class BaseDB(ABC):
    user_key = {
        "id": int,
        "account": str,
        "nickname": str,
        "password": str,
        "email": str,
        "phone": int,
        # "containers": list, # 废弃
        "photo": str,
    }
    image_key = {
        "id": int,
        "showname": str,
        "imagename": str,
        "init_args": dict,  # {参数: 默认值（可能为字典或数组）}
        "description": dict,  # 可以考虑 字符串 或 字典
    }
    container_key = {
        "id": int,
        "imageid": int,
        "machineid": int,
        "userid": int,
        "showname": str,
        "containername": str,  # 容器名为u{user account}_c{showname(random string)}
        "portlist": list,
        "running": bool,
    }
    machine_key = {
        "id": int,
        "ip": str,
        "machine_info": dict,  # {"gpu": dict, "cpu": dict, "disk": dict, "memory": dict}
        # {0：{"tpye":"3060","memory":10240}}
        # {"type":"i7-10700","core":8}
        # {"total":1024,"free":512}
        # {"total":1024,"free":512}
        "online": bool,
    }
    features_key = {
        "id": int,
        "name": str,
        "enabled": bool,
    }

    @abstractmethod
    def insert_user(self, user: dict) -> bool:
        pass

    @abstractmethod
    def get_user(
        self, search_key: dict, return_key: list = None, limit: int = None
    ) -> list:
        pass

    @abstractmethod
    def update_user(self, search_key: dict, update_key: dict) -> bool:
        pass

    @abstractmethod
    def delete_user(self, search_key: dict) -> bool:
        pass

    @abstractmethod
    def insert_image(self, image: dict) -> bool:
        pass

    @abstractmethod
    def get_image(
        self, search_key: dict, return_key: list = None, limit: int = None
    ) -> list:
        pass

    @abstractmethod
    def update_image(self, search_key: dict, update_key: dict) -> bool:
        pass

    @abstractmethod
    def delete_image(self, search_key: dict) -> bool:
        pass

    @abstractmethod
    def insert_container(self, container: dict) -> bool:
        pass

    @abstractmethod
    def get_container(
        self, search_key: dict, return_key: list = None, limit: int = None
    ) -> list:
        pass

    @abstractmethod
    def update_container(self, search_key: dict, update_key: dict) -> bool:
        pass

    @abstractmethod
    def delete_container(self, search_key: dict) -> bool:
        pass

    @abstractmethod
    def insert_machine(self, machine: dict) -> bool:
        pass

    @abstractmethod
    def get_machine(
        self, search_key: dict, return_key: list = None, limit: int = None
    ) -> list:
        pass

    @abstractmethod
    def update_machine(self, search_key: dict, update_key: dict) -> bool:
        pass

    @abstractmethod
    def delete_machine(self, search_key: dict) -> bool:
        pass

    @abstractmethod
    def all_user(self, return_key: list = None, limit: int = None) -> list:
        pass

    @abstractmethod
    def all_image(self, return_key: list = None, limit: int = None) -> list:
        pass

    @abstractmethod
    def all_container(self, return_key: list = None, limit: int = None) -> list:
        pass

    @abstractmethod
    def all_machine(self, return_key: list = None, limit: int = None) -> list:
        pass
