import yaml
from flask import Flask, g
from flask_cors import CORS

import communication
import database.SqliteDB
import dispatch.SchedulingStrategy as SS
from communication import BaseServer, DockerController
from database import BaseDB, InfoCache
from dispatch import WaitQueue
from WebServerAuth import auth

app = Flask(__name__)
app.register_blueprint(auth, url_prefix="/api/auth")
CORS(app)


def data_handler_gpus(info: dict, machine_id: int | str):
    g.gpus_cache.update(machine_id, info)


data_handler_funcs = {
    'gpus': data_handler_gpus,
}

data_handler = lambda data, machine_id: data_handler_funcs[data['type']](data['data'], machine_id)


def connect_handler(info, ip: tuple):
    db: BaseDB = g.db
    machine_id = info['machine_id']
    gpus = info['gpus']
    machines = db.get_machine(search_key={'id': machine_id})
    info = {'gpus': gpus}
    if len(machines) == 0:
        db.insert_machine(machine={'id': machine_id, 'ip': ip, 'machine_info': info})
    else:
        db.update_machine(search_key={'id': machine_id}, update_key={'machine_info': info, 'ip': ip})
    g.wq.new_machine(machine_id, {i: True for i in range(len(gpus))})
    return info


def run_handler(machine_id: int | str, task: dict):
    messenger: BaseServer = g.messenger
    messenger.send({'type': 'task', 'data': task}, machine_id)


disconnect_handler = lambda machine_id: g.wq.remove_machine(machine_id)

with app.app_context():
    configs = yaml.load(open('config.yaml'), Loader=yaml.FullLoader)
    g.repository = configs['Docker']['repository']
    DB_Class = getattr(database, configs['Database']['Class'])
    g.db: BaseDB = DB_Class(db_path=configs['Database']['db_path'])
    Scheduler_Class = getattr(SS, configs['Dispatch']['Class'])
    g.wq = WaitQueue(Scheduler_Class(**configs['Dispatch']['args']), run_handler)

    Messenger_Class = getattr(communication, configs['Components']['WebMessenger']['Class'])
    g.messenger: BaseServer = Messenger_Class(**configs['Components']['WebMessenger']['args'], data_handler=data_handler, connect_handler=connect_handler, disconnect_handler=disconnect_handler, logger=print)
    g.max_task_id = 0
    g.gpus_cache = InfoCache()
    g.docker = DockerController()
    if configs['Components']['Mail']['enable']:
        import communication.MailBox as MB
        Mail_Class = getattr(MB, configs['Components']['Mail']['Class'])
        g.mail = Mail_Class(**configs['Components']['Mail']['args'])


@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"



if __name__ == "__main__":
    app.run(host='0.0.0.0', port=9998, debug=True, threaded=True)
