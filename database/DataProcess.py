import pickle


USER = "(id INTEGER PRIMARY KEY AUTOINCREMENT, username VARCHAR (30) NOT NULL,realname TEXT,password TEXT,email VARCHAR (30), phone INTEGER, containers BLOB, photo TEXT)"
IMAGE = "(id INTEGER PRIMARY KEY, showname TEXT NOT NULL, imagename TEXT NOT NULL,tag TEXT NOT NULL,init_args BLOB,description TEXT)"
CONTAINER = "(id INTEGER PRIMARY KEY, showname TEXT NOT NULL, containername TEXT NOT NULL, machineid INTEGER NOT NULL, portlist BLOB, image INTERGER)"
MACHINE = "(id INTEGER PRIMARY KEY, ip TEXT, machine_info TEXT)"
FEATURE = "(id INTEGER PRIMARY KEY, name TEXT, enabled BOOL)"

TABEL_INFO = {"user": USER, "image": IMAGE, "container": CONTAINER, "machine": MACHINE, "feature": FEATURE}


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
