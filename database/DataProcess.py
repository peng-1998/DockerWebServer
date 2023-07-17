import pickle


USER = "(id INTEGER PRIMARY KEY AUTOINCREMENT, account VARCHAR (30) NOT NULL,nickname TEXT, password TEXT, email VARCHAR (30), phone INTEGER, photo TEXT)"
CONTAINER = "(id INTEGER PRIMARY KEY AUTOINCREMENT, showname TEXT NOT NULL, containername TEXT NOT NULL, machineid INTEGER NOT NULL, portlist BLOB, running BOOLEAN, imageid INTERGER NOT NULL, userid INTERGER Not NULL, FOREIGN KEY (imageid) REFERENCES image(id), FOREIGN KEY (machineid) REFERENCES machine(id),  FOREIGN KEY (userid) REFERENCES user(id))"
IMAGE = "(id INTEGER PRIMARY KEY AUTOINCREMENT, showname TEXT NOT NULL, imagename TEXT NOT NULL,tag TEXT NOT NULL,init_args BLOB,description TEXT)"
MACHINE = "(id INTEGER PRIMARY KEY, ip TEXT, machine_info TEXT)"
FEATURE = "(id INTEGER PRIMARY KEY, name TEXT, enabled BOOL)"

TABEL_INFO = {
    "user": USER,
    "image": IMAGE,
    "container": CONTAINER,
    "machine": MACHINE,
    "feature": FEATURE,
}


def serialize_data(data):
    return pickle.dumps(data)


def deserialize_data(data):
    return pickle.loads(data)


def check_and_serialize(table_name: str, search_key: dict) -> list:
    assert table_name in TABEL_INFO.keys()
    serialized_key_list = ["init_args", "portlist", "machine_info"]
    keys_to_deserialize_list = []
    for key in serialized_key_list:
        if key in search_key.keys():
            search_key[key] = serialize_data(search_key[key])
            keys_to_deserialize_list.append(key)
    return keys_to_deserialize_list
