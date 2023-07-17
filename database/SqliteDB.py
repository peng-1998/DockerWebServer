import sqlite3
from .BaseDB import BaseDB
from warnings import warn
from .DataProcess import check_and_serialize, deserialize_data, TABEL_INFO


class SQLiteDB(BaseDB):
    "Mapping class to table"

    def __init__(self, db_path: str) -> None:
        super().__init__()
        warn(
            "This class wil be deprecated in the future. Please use Flask-SQLAlchemy instead."
        )
        self.db = sqlite3.connect(db_path)
        # 数据库初始化建表，如果不存在则创建
        self.cursor = self.db.cursor()
        for table_name in TABEL_INFO.keys():
            self._create_table(table_name=table_name)
        self.db.commit()

    # 线程安全的数据库操作，在API操作完成之后，关闭连接。
    @staticmethod
    def runthenclose(func):
        def wrapper(self, *args, **kwargs):
            func(self, *args, **kwargs)
            self.cursor.close()
            self.db.close()

        return wrapper

    def _create_table(self, table_name: str):
        assert table_name in TABEL_INFO.keys()
        self.cursor.execute(
            f"CREATE TABLE IF NOT EXISTS {table_name} {TABEL_INFO[table_name]}"
        )

    @runthenclose
    def _insert(self, table_name: str, data_dict: dict):
        try:
            check_and_serialize(table_name, data_dict)
            self.cursor.execute(
                f"INSERT INTO {table_name.lower()} ({', '.join([f'{k}' for k in data_dict.keys()])}) VALUES ({', '.join(['?' for k in data_dict.keys()])})",
                [v for v in data_dict.values()],
            )
            self.db.commit()
            return True
        except:
            return False

    @runthenclose
    def _delete(self, table_name: str, search_key: dict):
        try:
            self.cursor.execute(
                f"DELETE FROM {table_name.lower()} WHERE {' AND '.join([f'{k} = ?' for k in search_key.keys()])}",
                [v for v in search_key.values()],
            )
            self.db.commit()
            return True
        except:
            return False

    @runthenclose
    def _get(
        self,
        table_name: str,
        search_key: dict,
        return_key: list = None,
        limit: int = None,
    ):
        if return_key is None:
            return_key = ["*"]
        if limit is None:
            limit = 100
        self.cursor.execute(
            f"SELECT {', '.join(return_key)} FROM {table_name.lower()} WHERE {f' AND '.join([f'{k} = ?' for k in search_key.keys()])} LIMIT ?",
            tuple([v for v in search_key.values()] + [limit]),
        )
        info = self.cursor.fetchall()
        return info

    @runthenclose
    def _update(self, table_name: str, search_key: dict, update_key: dict):
        self.cursor.execute(
            f"UPDATE {table_name.lower()} SET {', '.join([f'{k} = ?' for k in update_key.keys()])} WHERE {f' AND '.join([f'{k} = ?' for k in search_key.keys()])}",
            [v for v in update_key.values()] + [v for v in search_key.values()],
        )
        self.db.commit()

    def get_user(
        self, search_key: dict, return_key: list = None, limit: int = None
    ) -> list:
        return self._get("user", search_key, return_key, limit)

    def get_image(
        self, search_key: dict, return_key: list = None, limit: int = None
    ) -> list:
        return self._get("image", search_key, return_key, limit)

    def get_container(
        self, search_key: dict, return_key: list = None, limit: int = None
    ) -> list:
        return self._get("container", search_key, return_key, limit)

    def get_machine(
        self, search_key: dict, return_key: list = None, limit: int = None
    ) -> list:
        return self._get("machine", search_key, return_key, limit)

    def get_feature(
        self, search_key: dict, return_key: list = None, limit: int = None
    ) -> list:
        return self._get("feature", search_key, return_key, limit)

    def insert_user(self, user: dict) -> bool:
        return self._insert("user", user)

    def insert_image(self, image: dict) -> bool:
        return self._insert("image", image)

    def insert_container(self, container: dict) -> bool:
        return self._insert("container", container)

    def insert_machine(self, machine: dict) -> bool:
        return self._insert("machine", machine)

    def insert_feature(self, feature: dict) -> bool:
        return self._insert("feature", feature)

    def delete_user(self, search_key: dict) -> bool:
        return self._delete("user", search_key)

    def delete_image(self, search_key: dict) -> bool:
        return self._delete("image", search_key)

    def delete_container(self, search_key: dict) -> bool:
        return self._delete("container", search_key)

    def delete_machine(self, search_key: dict) -> bool:
        return self._delete("machine", search_key)

    def delete_feature(self, search_key: dict) -> bool:
        return self._delete("feature", search_key)

    def update_user(self, search_key: dict, update_key: dict) -> bool:
        return self._update("user", search_key, update_key)

    def update_image(self, search_key: dict, update_key: dict) -> bool:
        return self._update("image", search_key, update_key)

    def update_container(self, search_key: dict, update_key: dict) -> bool:
        return self._update("container", search_key, update_key)

    def update_machine(self, search_key: dict, update_key: dict) -> bool:
        return self._update("machine", search_key, update_key)

    @runthenclose
    def all_user(self) -> list:
        self.cursor.execute("SELECT * FROM user")
        return self.cursor.fetchall()

    @runthenclose
    def all_image(self) -> list:
        self.cursor.execute("SELECT * FROM image")
        return self.cursor.fetchall()

    @runthenclose
    def all_container(self) -> list:
        self.cursor.execute("SELECT * FROM container")
        return self.cursor.fetchall()

    @runthenclose
    def all_machine(self) -> list:
        self.cursor.execute("SELECT * FROM machine")
        return self.cursor.fetchall()
