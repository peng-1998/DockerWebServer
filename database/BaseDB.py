from abc import ABC, abstractmethod
"""
Table User {
    id int [pk, increment]
    username varchar
    realname varchar optional
    password varchar
    email varchar optional
    phone varchar optional
    containers list [ref: > Container.id]
}
Table Image {
    id int [pk, increment]
    showname varchar
    imagename varchar
    tag varchar
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
        "containers": list,
        "photo": str,
    }
    image_key = {
        "id": int,
        "showname": str,
        "imagename": str,
        "tag": str,
        "init_args": dict,
        "description": dict,
    }
    container_key = {
        "id": int,
        "showname": str,
        "containername": str,
        "machineid": int,
        "portlist": list,
        "image": int,
    }
    machine_key = {"id": int, "ip": str, "machine_info": dict}

    @abstractmethod
    def insert_user(self, user: dict) -> bool:
        pass

    @abstractmethod
    def get_user(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
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
    def get_image(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
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
    def get_container(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
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
    def get_machine(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        pass

    @abstractmethod
    def update_machine(self, search_key: dict, update_key: dict) -> bool:
        pass

    @abstractmethod
    def delete_machine(self, search_key: dict) -> bool:
        pass
