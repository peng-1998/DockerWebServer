import pickle


USER = "(id INTEGER PRIMARY KEY AUTOINCREMENT, account VARCHAR (30) NOT NULL,nickname TEXT, password TEXT, email VARCHAR (30), phone INTEGER, photo TEXT)"
IMAGE = "(id INTEGER PRIMARY KEY AUTOINCREMENT, showname TEXT NOT NULL, imagename TEXT NOT NULL, init_args BLOB, description TEXT)"
CONTAINER = "(id INTEGER PRIMARY KEY AUTOINCREMENT, showname TEXT NOT NULL, containername TEXT NOT NULL, machineid INTEGER NOT NULL, portlist BLOB, running BOOLEAN, imageid INTERGER FOREIGN KEY REFERENCES image(id), machineid INTEGER FOREIGN KEY REFERENCES machine(id), userid INTEGER FOREIGN KEY REFERENCES user(id))"
MACHINE = "(id INTEGER PRIMARY KEY AUTOINCREMENT, ip TEXT, gpu BLOB, disk BLOB, cpu BLOB, memory BLOB, online BOOLEAN)"

TABEL_INFO = {"user": USER, "image": IMAGE, "container": CONTAINER, "machine": MACHINE}


def serialize_data(data):
    return pickle.dumps(data)


def deserialize_data(data):
    return pickle.loads(data)


def check_and_serialize(table_name: str, search_key: dict):
    assert table_name in TABEL_INFO.keys()
    if search_key.get("containers"):
        search_key["containers"] = serialize_data(search_key["containers"])
    if search_key.get("init_args"):
        search_key["init_args"] = serialize_data(search_key["init_args"])
    if search_key.get("portlist"):
        search_key["portlist"] = serialize_data(search_key["portlist"])
    if search_key.get("gpu"):
        search_key["gpu"] = serialize_data(search_key["gpu"])
    if search_key.get("disk"):
        search_key["disk"] = serialize_data(search_key["disk"])
    if search_key.get("cpu"):
        search_key["cpu"] = serialize_data(search_key["cpu"])
    if search_key.get("memory"):
        search_key["memory"] = serialize_data(search_key["memory"])
    if search_key.get("description"):
        search_key["description"] = serialize_data(search_key["description"])
