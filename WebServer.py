import yaml
from flask import Flask, g
from flask_cors import CORS

import communication
import database.SqliteDB
import dispatch.SchedulingStrategy as SS
from communication import BaseServer, DockerController
from communication.Auth import auth
from database import BaseDB, InfoCache
from dispatch import WaitQueue

app = Flask(__name__)
app.register_blueprint(auth, url_prefix="/api/auth")
CORS(app)


def data_handler_gpus(info: dict, machine_id: int | str):
    g.gpus_cache.update(machine_id, info)


# all functions get two parameters, the first is the data (json), the second is the machine_id
data_handler_funcs = {
    'gpus': data_handler_gpus,
}
# data: {'type': str, 'data': dict}
data_handler = lambda data, machine_id: data_handler_funcs[data['type']](data['data'], machine_id)
disconnect_handler = lambda machine_id: g.wq.remove_machine(machine_id)


def connect_handler(info, ip: str) -> dict:
    # the info is a dict, is the first data sent by the machine
    # info: {'machine_id': int, 'gpus': [{'type': str, 'memory': int, 'id': int}, ...],'cpu':{...},'memory':{...},'disk':{...},'url':str}
    db: BaseDB = g.db
    machine_id = info['machine_id']
    del info['machine_id']
    gpus = info['gpus']
    machines = db.get_machine(search_key={'id': machine_id})
    if len(machines) == 0:
        db.insert_machine(machine={'id': machine_id, 'ip': ip, 'machine_info': info})
    else:
        db.update_machine(search_key={'id': machine_id}, update_key={'machine_info': info, 'ip': ip})
    g.wq.new_machine(machine_id, {i: True for i in range(len(gpus))})
    return info


def run_handler(machine_id: int | str, task: dict) -> None:
    messenger: BaseServer = g.messenger
    messenger.send({'type': 'task', 'data': task}, machine_id)


with app.app_context():
    with open('config.yaml') as f:
        configs = yaml.load(f, Loader=yaml.FullLoader)
    if configs['Components']['Logger']['enable']:
        from database import Logger
        g.logger = Logger(**configs['Components']['Logger']['args'])
    else:
        g.logger = print
    g.repository = configs['Docker']['repository']
    DB_Class = getattr(database, configs['Database']['Class'])
    g.db: BaseDB = DB_Class(db_path=configs['Database']['db_path'])
    Scheduler_Class = getattr(SS, configs['Dispatch']['Class'])
    g.wq = WaitQueue(Scheduler_Class(**configs['Dispatch']['args']), run_handler, g.logger)
    Messenger_Class = getattr(communication, configs['Components']['WebMessenger']['Class'])
    g.messenger: BaseServer = Messenger_Class(**configs['Components']['WebMessenger']['args'], data_handler=data_handler, connect_handler=connect_handler, disconnect_handler=disconnect_handler, logger=g.logger)
    g.messenger.start()  # start the messenger thread
    g.max_task_id = 0
    g.gpus_cache = InfoCache()
    g.docker = DockerController()
    if configs['Components']['Mail']['enable']:
        import communication.MailBox as MB
        Mail_Class = getattr(MB, configs['Components']['Mail']['Class'])
        g.mail = Mail_Class(**configs['Components']['Mail']['args'], logger=g.logger)


@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=10000, debug=False, threaded=True)
    # 目前有个bug,如果以debug模式运行,会导致套接字被定义两次,导致第二次无法绑定端口
