import os
from tinydb import TinyDB, Query
from .BaseDB import BaseDB



class DB_TinyDB(BaseDB):

    def __init__(self, db_path: str):
        super().__init__(db_path)
        self.db = TinyDB(db_path, sort_keys=True, indent=4)
        self.user_table = self.db.table('User')
        self.image_table = self.db.table('Image')
        self.container_table = self.db.table('Container')
        self.machine_table = self.db.table('Machine')
        self.user_table.purge()
        self.query = Query()

    def insert_user(self, user: dict) -> bool:
        for key in self.user_key:
            if key not in user:
                if 
        user_id = self.user_table.insert(user)
        return bool(user_id)

    def get_user(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        users = self.user_table.search(self.query.fragment(search_key))
        if return_key:
            users = [{key: user[key] for key in return_key} for user in users]
        if limit:
            users = users[:limit]
        return users

    def update_user(self, search_key: dict, update_key: dict) -> bool:
        return bool(self.user_table.update(update_key, self.query.fragment(search_key)))

    def delete_user(self, search_key: dict) -> bool:
        return bool(self.user_table.remove(self.query.fragment(search_key)))

    def insert_image(self, image: dict) -> bool:
        pass

    def get_image(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        pass

    def update_image(self, search_key: dict, update_key: dict) -> bool:
        pass

    def delete_image(self, search_key: dict) -> bool:
        pass

    def insert_container(self, container: dict) -> bool:
        pass

    def get_container(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        pass

    def update_container(self, search_key: dict, update_key: dict) -> bool:
        pass

    def delete_container(self, search_key: dict) -> bool:
        pass

    def insert_machine(self, machine: dict) -> bool:
        pass

    def get_machine(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        pass

    def update_machine(self, search_key: dict, update_key: dict) -> bool:
        pass

    def delete_machine(self, search_key: dict) -> bool:
        pass