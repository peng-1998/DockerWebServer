import sqlite3
from .BaseDB import BaseDB
import pickle

USER = "(user_id INTEGER PRIMARY KEY AUTOINCREMENT, username VARCHAR (30) NOT NULL,realname TEXT,password TEXT,email VARCHAR (30), phone INTEGER,containers BLOB, photo TEXT)"
IMAGE = "(image_id INTEGER PRIMARY KEY, showname TEXT NOT NULL, imagename TEXT NOT NULL,tag TEXT NOT NULL,init_args BLOB,description BLOB)"
CONTAINER = "(container_id INTEGER PRIMARY KEY, showname TEXT NOT NULL, containername TEXT NOT NULL, machine_id INTEGER NOT NULL, portlist BLOB, image INTERGER)"
MACHINE = "(machine_id INTEGER PRIMARY KEY, ip INTEGER, MACHINE_INFO TEXT)"

TABEL_INFOR = {"user": USER, "image": IMAGE, "container": CONTAINER, "machine": MACHINE}


def serialize_data(data):
    return pickle.dump(data)


class SqlLiteDB(BaseDB):
    def __init__(self, db_path: str) -> None:
        super().__init__()
        self.db = sqlite3.connect(db_path)
        # 数据库初始化建表，如果不存在则创建
        self.cursor = self.db.cursor()
        for table_name in TABEL_INFOR.keys():
            self._create_table(table_name=table_name)
        self.db.commit()

    def _create_table(self, table_name: str):
        assert table_name in TABEL_INFOR.keys()
        self.cursor.execute(
            f"CREATE TABLE IF NOT EXISTS {table_name} {TABEL_INFOR[table_name]}"
        )

    def _insert(self, table_name: str, data_dict: dict):
        self.cursor.execute(
            f"INSERT INTO {table_name.lower()} ({', '.join([f'{k}' for k in data_dict.keys()])}) VALUES ({', '.join(['?' for k in data_dict.keys()])})",
            (v for v in data_dict.values()),
        )
        self.db.commit()

    def _delete(self, table_name: str, search_key: dict):
        self.cursor.execute(
            f"DELETE FROM {table_name.lower()} WHERE {' AND '.join([f'{k} = ?' for k in search_key.keys()])}",
            (v for v in search_key.values()),
        )
        self.db.commit()
