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

    def _insertitem_to_table(self, table: str, item: dict, keys: dict) -> bool:
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
        item_id = self.db.table(table).insert(item)
        self.db.table(table).update({'id': item_id}, doc_ids=item_id)
        return bool(item_id)

    def _getitem_from_table(self, table: str, search_key: dict, return_key: list = None, limit: int = None) -> list:
        items = self.db.table(table).search(self.query.fragment(search_key))
        if return_key:
            items = [{key: item[key] for key in return_key} for item in items]
        if limit:
            items = items[:limit]
        return items

    def _updateitem_from_table(self, table: str, search_key: dict, update_key: dict) -> bool:
        return bool(self.db.table(table).update(update_key, self.query.fragment(search_key)))

    def _deleteitem_from_table(self, table: str, search_key: dict) -> bool:
        return bool(self.db.table(table).remove(self.query.fragment(search_key)))

    def insert_user(self, user: dict) -> bool:
        return self._insertitem_to_table('User', user, BaseDB.user_key)

    def get_user(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        return self._getitem_from_table('User', search_key, return_key, limit)

    def update_user(self, search_key: dict, update_key: dict) -> bool:
        return self._updateitem_from_table('User', search_key, update_key)

    def delete_user(self, search_key: dict) -> bool:
        return self._deleteitem_from_table('User', search_key)

    def insert_image(self, image: dict) -> bool:
        return self._insertitem_to_table('Image', image, BaseDB.image_key)

    def get_image(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        return self._getitem_from_table('Image', search_key, return_key, limit)

    def update_image(self, search_key: dict, update_key: dict) -> bool:
        return self._updateitem_from_table('Image', search_key, update_key)

    def delete_image(self, search_key: dict) -> bool:
        return bool(self.image_table.remove(self.query.fragment(search_key)))

    def insert_container(self, container: dict) -> bool:
        return self._insertitem_to_table('Container', container, BaseDB.container_key)
 
    def get_container(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        return self._getitem_from_table('Container', search_key, return_key, limit)

    def update_container(self, search_key: dict, update_key: dict) -> bool:
        return self._updateitem_from_table('Container', search_key, update_key)

    def delete_container(self, search_key: dict) -> bool:
        return self._deleteitem_from_table('Container', search_key)

    def insert_machine(self, machine: dict) -> bool:
        for key, value in self.machine_key.items():
            if key not in machine:
                if value == str:
                    machine[key] = ''
                elif value == dict:
                    machine[key] = {}
                elif value == list:
                    machine[key] = []
                elif value == int:
                    machine[key] = 0
        item_id = self.db.table('Machine').insert(machine)
        return bool(item_id)

    def get_machine(self, search_key: dict, return_key: list = None, limit: int = None) -> list:
        return self._getitem_from_table('Machine', search_key, return_key, limit)

    def update_machine(self, search_key: dict, update_key: dict) -> bool:
        return self._updateitem_from_table('Machine', search_key, update_key)

    def delete_machine(self, search_key: dict) -> bool:
        return self._deleteitem_from_table('Machine', search_key)

