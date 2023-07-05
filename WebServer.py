import os

import yaml
from flask import Flask, g, jsonify, request
from flask_cors import CORS
from flask_jwt_extended import JWTManager, verify_jwt_in_request
from flask_jwt_extended.exceptions import NoAuthorizationError

import communication
import database.SqliteDB
import dispatch.SchedulingStrategy as SS
from blueprints import admin, auth, containers, machines, user
from communication import BaseServer, DockerController
from database import BaseDB, InfoCache
from dispatch import WaitQueue

os.chdir(os.path.dirname(__file__))
print(os.getcwd())

app = Flask(__name__)
app.register_blueprint(auth, url_prefix="/api/auth")
app.register_blueprint(admin, url_prefix="/api/admin")
app.register_blueprint(containers, url_prefix="/api/containers")
app.register_blueprint(machines, url_prefix="/api/machines")
app.register_blueprint(user, url_prefix="/api/user")

app.config['JWT_SECRECT_KEY'] = 'mycreditentials'
jwt = JWTManager(app)
CORS(app)


def data_handler_gpus(info: dict, machine_id: int | str):
    """ deal with the gpu info sent by the machine

    Args:
        info (dict): the gpu info sent by the machine. e.g. {'0': {'type': 'Nvidia RTX 3060Ti', 'memory_total': 10240, 'memory_used': 2048, 'utilization': 0.96 }, ...}
        machine_id (int | str): the id of the machine
    """
    g.gpus_cache.update(machine_id, info)


# all functions get two parameters, the first is the data (json), the second is the machine_id
data_handler_funcs = {
    'gpus': data_handler_gpus,
}
# data: {'type': str, 'data': dict}
data_handler = lambda data, machine_id: data_handler_funcs[data['type']](data['data'], machine_id)
disconnect_handler = lambda machine_id: g.wq.remove_machine(machine_id)


def connect_handler(info: dict, ip: str) -> dict:
    """ deal with the first data sent by the machine

    Args:
        info (dict): the info sent by the machine. e.g. 
            {'machine_id': 1, 
                'gpus': {0: {'type': 'NVIDIA GeForce RTX 3060', 'memory_total': 12288.0, 'memory_used': 3187.25, 'utilization': 45}}, 
                'cpu': {'type': ' Intel(R) Core(TM) i5-10400F CPU @ 2.90GHz', 'logical_cpu_count': 12, 'physical_cpu_count': 6},
                'memory': {'total': 62.71, 'free': 30.26}, 
                'disk': {'total': 195.8, 'free': 112.51}, 
                'url': 'www.sdasds.com'}
        ip (str): the ip of the machine

    Returns:
        dict: the info return to Messenger
    """
    db: BaseDB = g.db
    machine_id = info['machine_id']
    del info['machine_id']
    gpus = info['gpus']
    machines_list = db.get_machine(search_key={'id': machine_id})
    if len(machines_list) == 0:
        db.insert_machine(machine={'id': machine_id, 'ip': ip, 'machine_info': info})
    else:
        db.update_machine(search_key={'id': machine_id}, update_key={'machine_info': info, 'ip': ip})
    g.wq.new_machine(machine_id, {i: True for i in range(len(gpus))})
    return {'machine_id': machine_id}


def run_handler(machine_id: int | str, task: dict) -> None:
    messenger: BaseServer = g.messenger
    messenger.send({'type': 'task', 'data': task}, machine_id)


with app.app_context():
    with open('WebServerConfig.yaml') as f:
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
    g.messenger: BaseServer = Messenger_Class(**configs['Components']['WebMessenger']['args'], data_handler=data_handler, connect_handler=connect_handler, disconnect_handler=disconnect_handler)
    g.messenger.start()  # start the messenger thread
    g.max_task_id = 0
    g.gpus_cache = InfoCache()
    g.docker = DockerController()
    if configs['Components']['Mail']['enable']:
        import communication.MailBox as MB
        Mail_Class = getattr(MB, configs['Components']['Mail']['Class'])
        g.mail = Mail_Class(**configs['Components']['Mail']['args'], logger=g.logger)


@app.before_request
def is_jwt_valid():
    """ 
    check if the jwt is valid, if not, return 401
    except the login and register request
    """
    if request.endpoint in ['login', 'register']:
        return
    try:
        verify_jwt_in_request()
    except NoAuthorizationError:
        return jsonify({'message': 'Invalid token'}, 401)


@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=10000, debug=False, threaded=True)
    # 目前有个bug,如果以debug模式运行,会导致套接字被定义两次,导致第二次无法绑定端口
