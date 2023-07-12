from tinydb import TinyDB, Query
from .BaseDB import BaseDB
from itertools import islice
from readerwriterlock import rwlock

class TinyDB_(BaseDB):

    def __init__(self, db_path: str):
        super().__init__()
        self.db = TinyDB(db_path, sort_keys=True, indent=4)
        self.user_table = self.db.table('User')
        self.image_table = self.db.table('Image')
        self.container_table = self.db.table('Container')
        self.machine_table = self.db.table('Machine')
        self.query = Query()
        self.rw_lock = rwlock.RWLockWrite()

    def __insert_item(self, table: str, item: dict, keys: dict) -> bool:
        item = self.__setdefault(item, keys)
        item_id = self.db.table(table).insert(item)
        with self.rw_lock.gen_wlock():
            self.db.table(table).update({'id': item_id}, doc_ids=[item_id])
        return bool(item_id)

    def __get_item(self, table: str, search_key: dict, return_key: list = None, limit: int = None) -> list:
        with self.rw_lock.gen_rlock():
            items = self.db.table(table).search(self.query.fragment(search_key))
        if return_key:
            items = [{key: item[key] for key in return_key} for item in items]
        if limit:
            items = islice(items, limit)
        return items

    def __update_item(self, table: str, search_key: dict, update_key: dict) -> bool:
        with self.rw_lock.gen_wlock():
            res = self.db.table(table).update(update_key, self.query.fragment(search_key))
        return bool(res)

    def __delete_item(self, table: str, search_key: dict) -> bool:
        with self.rw_lock.gen_wlock():
            res = self.db.table(table).remove(self.query.fragment(search_key))
        return bool(res)

    def __setdefault(self, item: dict, keys: dict) -> dict:
        for key, value in keys.items():
            if key not in item:
                if value == str:
                    item[key] = ''
                elif value == dict:
                    item[key] = {}
                elif value == list:
                    item[key] = []
                elif value == int:
                    item[key] = 0
                elif value == bool:
                    item[key] = False
        return item

    def __all_item(self, table: str, return_key: list = None, limit: int = None) -> list:
        with self.rw_lock.gen_rlock():
            items = self.db.table(table).all()
        if return_key:
            items = [{key: item[key] for key in return_key} for item in items]
        if limit:
            items = islice(items, limit)
        return items

    def insert_user(self, user: dict) -> bool:
        return self.__insert_item('User', user, BaseDB.user_key)

    def get_user(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        return self.__get_item('User', search_key, return_key, limit)

    def update_user(self, search_key: dict, update_key: dict) -> bool:
        return self.__update_item('User', search_key, update_key)

    def delete_user(self, search_key: dict) -> bool:
        return self.__delete_item('User', search_key)

    def insert_image(self, image: dict) -> bool:
        return self.__insert_item('Image', image, BaseDB.image_key)

    def get_image(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        return self.__get_item('Image', search_key, return_key, limit)

    def update_image(self, search_key: dict, update_key: dict) -> bool:
        return self.__update_item('Image', search_key, update_key)

    def delete_image(self, search_key: dict) -> bool:
        return bool(self.image_table.remove(self.query.fragment(search_key)))

    def insert_container(self, container: dict) -> bool:
        return self.__insert_item('Container', container, BaseDB.container_key)
 
    def get_container(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        return self.__get_item('Container', search_key, return_key, limit)

    def update_container(self, search_key: dict, update_key: dict) -> bool:
        return self.__update_item('Container', search_key, update_key)

    def delete_container(self, search_key: dict) -> bool:
        return self.__delete_item('Container', search_key)

    def insert_machine(self, machine: dict) -> bool:
        machine = self.__setdefault(machine, BaseDB.machine_key)
        item_id = self.db.table('Machine').insert(machine)
        return bool(item_id)

    def get_machine(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        return self.__get_item('Machine', search_key, return_key, limit)

    def update_machine(self, search_key: dict, update_key: dict) -> bool:
        return self.__update_item('Machine', search_key, update_key)

    def delete_machine(self, search_key: dict) -> bool:
        return self.__delete_item('Machine', search_key)
    
    def all_user(self, return_key: list = None, limit: int = None) -> list:
        return self.__all_item('User', return_key, limit)
    
    def all_image(self, return_key: list = None, limit: int = None) -> list:
        return self.__all_item('Image', return_key, limit)
    
    def all_container(self, return_key: list = None, limit: int = None) -> list:
        return self.__all_item('Container', return_key, limit)
    
    def all_machine(self, return_key: list = None, limit: int = None) -> list:
        return self.__all_item('Machine', return_key, limit)

