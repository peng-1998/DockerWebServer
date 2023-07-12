import json
import os

import yaml
from flask import Flask, jsonify, request, make_response, g
from flask_cors import CORS
from flask_jwt_extended import JWTManager, verify_jwt_in_request
from flask_jwt_extended.exceptions import NoAuthorizationError

import communication
import database.SqliteDB
import dispatch.SchedulingStrategy as SS
from blueprints import admin, auth, containers, machines, user, feature

from communication import BaseServer, DockerController
from database import BaseDB, InfoCache
from dispatch import WaitQueue
from queue import Queue

os.chdir(os.path.dirname(__file__))

app = Flask(__name__)
app.register_blueprint(auth, url_prefix="/api/auth")
app.register_blueprint(admin, url_prefix="/api/admin")
app.register_blueprint(containers, url_prefix="/api/containers")
app.register_blueprint(machines, url_prefix="/api/machines")
app.register_blueprint(user, url_prefix="/api/user")
app.register_blueprint(feature, url_prefix="api/feature")

app.config['JWT_SECRECT_KEY'] = 'mycreditentials'
jwt = JWTManager(app)
CORS(app)


def data_handler_gpus(info: dict, machine_id: int | str):
    """ deal with the gpu info sent by the machine

    Args:
        info (dict): the gpu info sent by the machine. e.g. {'0': {'type': 'Nvidia RTX 3060Ti', 'memory_total': 10240, 'memory_used': 2048, 'utilization': 0.96 }, ...}
        machine_id (int | str): the id of the machine
    """
    app.config['gpus_cache'].update(machine_id, info)


def data_handler_image(data: dict, machine_id: int | str):
    if data['status'] == 'failed':
        app.config['error_logger'](f'{machine_id} {data["opt"]} image:{data["image"]} failed, error info: {data["error"]}')
        app.config['info_logger'](f'{machine_id} {data["opt"]} image:{data["image"]} failed')
        return
    app.config['info_logger'](f'{machine_id} {data["opt"]} image:{data["image"]} success')
    if data['opt'] == 'pull':

        ...
    elif data['opt'] == 'remove':

        ...


def data_handler_container(data: dict, machine_id: int | str):
    user_id = data['user_id']
    massage_cache: InfoCache = app.config['massage_cache']
    if massage_cache.contains(user_id):
        massage_queue: Queue = Queue()
    else:
        massage_queue: Queue = massage_cache.get(user_id)
    massage_queue.put({'type': 'container', 'opt': data['opt'], "status": data["status"]})
    massage_cache.update(user_id, massage_queue)

    db: BaseDB = app.config['db']
    app.config['info_logger'](f'{machine_id} {data["opt"]} container:{data["container"]} success')
    if data['opt'] == 'create':
        if data['status'] == 'failed':
            app.config['error_logger'](f'{machine_id} {data["opt"]} container:{data["container"]} failed, error info: {data["error"]} create args: {json.dumps(data["original_data"])}')
            app.config['info_logger'](f'{machine_id} {data["opt"]} container:{data["container"]} failed')
            return
        app.config['info_logger'](f'{machine_id} {data["opt"]} container:{data["container"]} success')
        original_data = data['original_data']
        showname = original_data["name"].split('_')[-1][1:]
        portlist = [[int(k.split('/')[0]), v] for k, v in original_data["ports"].items()]
        image_id = db.get_image(search_key={"imagename": original_data["image"]}, return_key=['id'])[0]["id"]
        db.insert_container({"showname": showname, "containername": data["name"], "userid": user_id, "machineid": machine_id, "portlist": portlist, "image": image_id, "runing": False})
        return
    if data['status'] == 'failed':
        app.config['error_logger'](f'{machine_id} {data["opt"]} container:{data["container"]} failed, error info: {data["error"]}')
        app.config['info_logger'](f'{machine_id} {data["opt"]} container:{data["container"]} failed')
        return
    if data['opt'] == 'remove':
        db.delete_container(search_key={"containername": data["container"], "machineid": machine_id})
    elif data['opt'] in ['start', 'stop', 'restart']:
        db.update_container(search_key={"containername": data["container"], "machineid": machine_id}, update_key={"running": 'start' in data['opt']})


def data_handler_task(data: dict, machine_id: int | str):
    if data['status'] == 'failed' and data['opt'] == 'run':
        app.config['error_logger'](f'{machine_id} {data["opt"]} task failed, error info: {data["error"]}')
        app.config['info_logger'](f'{machine_id} {data["opt"]} task failed')
        app.config['wq'].finish_task(machine_id, data['task_id'])
        return


# all functions get two parameters, the first is the data (json), the second is the machine_id
data_handler_funcs = {'gpus': data_handler_gpus, 'image': data_handler_image, 'container': data_handler_container, 'task': data_handler_task}
# data: {'type': str, 'data': dict}
data_handler = lambda data, machine_id: data_handler_funcs[data['type']](data['data'], machine_id)
disconnect_handler = lambda machine_id: app.config['wq'].remove_machine(machine_id)


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
    db: BaseDB = app.config['db']
    machine_id = info['machine_id']
    del info['machine_id']
    gpus = info['gpus']
    machines_list = db.get_machine(search_key={'id': machine_id})
    if len(machines_list) == 0:
        db.insert_machine(machine={'id': machine_id, 'ip': ip, 'machine_info': info})
    else:
        db.update_machine(search_key={'id': machine_id}, update_key={'machine_info': info, 'ip': ip})
    app.config['wq'].new_machine(machine_id, {i: True for i in range(len(gpus))})
    containers = info['containers']
    for container in containers:
        db.update_container(search_key={'containername': container['name'], 'machineid': machine_id}, update_key={'running': container['running']})
    return {'machine_id': machine_id}


def run_finish_mail(task: dict):
    return "", ""


def run_handler(machine_id: int | str, task: dict) -> None:
    messenger: BaseServer = app.config['messenger']
    task['opt'] = 'run'
    messenger.send({'type': 'task', 'data': task}, machine_id)
    if hasattr(app.config, 'mail'):
        user_id = task['user_id']
        user = app.config['db'].get_user(search_key={'id': user_id}, return_key=[''])[0]
        if user['email'] != '':
            app.config['mail'].append(user['email'], user['nickname'], *run_finish_mail(task))


def finish_handler(machine_id: int | str, task: dict) -> None:
    messenger: BaseServer = app.config['messenger']
    task['opt'] = 'finish'
    messenger.send({'type': 'task', 'data': task}, machine_id)
    if hasattr(app.config, 'mail'):
        user_id = task['user_id']
        user = app.config['db'].get_user(search_key={'id': user_id}, return_key=[''])[0]
        if user['email'] != '':
            app.config['mail'].append(user['email'], user['nickname'], *run_finish_mail(task))


# 初始化
with app.app_context():
    with open('WebServerConfig.yaml') as f:
        configs = yaml.load(f, Loader=yaml.FullLoader)
    app.config['configs'] = configs
    if configs['Components']['Logger']['enable']:
        from database import Logger
        app.config['info_logger'] = Logger(**configs['Components']['Logger']['args'])
    else:
        app.config['info_logger'] = print
    if configs['Components']['ErrorLogger']['enable']:
        from database import Logger
        app.config['error_logger'] = Logger(**configs['Components']['ErrorLogger']['args'])
    else:
        app.config['error_logger'] = print

    app.config['repository'] = configs['Docker']['repository']
    DB_Class = getattr(database, configs['Database']['Class'])
    app.config['db']: BaseDB = DB_Class(db_path=configs['Database']['db_path'])
    Scheduler_Class = getattr(SS, configs['Dispatch']['Class'])
    app.config['wq'] = WaitQueue(Scheduler_Class(**configs['Dispatch']['args']), run_handler, app.config['info_logger'])
    app.config['wq'].start()
    Messenger_Class = getattr(communication, configs['Components']['WebMessenger']['Class'])
    app.config['messenger']: BaseServer = Messenger_Class(**configs['Components']['WebMessenger']['args'], data_handler=data_handler, connect_handler=connect_handler, disconnect_handler=disconnect_handler)
    app.config['messenger'].start()  # start the messenger thread
    g.max_task_id = 0
    app.config['gpus_cache'] = InfoCache()
    app.config['massage_cache'] = InfoCache()
    g.docker = DockerController()
    if configs['Components']['Mail']['enable']:
        import communication.MailBox as MB
        Mail_Class = getattr(MB, configs['Components']['Mail']['Class'])
        app.config['mail'] = Mail_Class(**configs['Components']['Mail']['args'], logger=app.config['info_logger'])


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


@app.route("/api/massage/<user_id>", methods=['GET'])
def get_massage(user_id: int):
    msg_queue: Queue = app.config['massage_cache'].get(user_id)
    if msg_queue is None or len(msg_queue) == 0:
        return make_response(jsonify({'message': 'no message'}), 404)
    msg = msg_queue.get()
    return make_response(jsonify(msg), 200)


@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=10000, debug=False, threaded=True)
    # 目前有个bug,如果以debug模式运行,会导致套接字被定义两次,导致第二次无法绑定端口
