import asyncio
import json
import os
from queue import Queue

import yaml
from quart import Quart, g, jsonify, request, websocket
from quart_cors import cors
from quart_jwt_extended import JWTManager, verify_jwt_in_request
from quart_jwt_extended.exceptions import NoAuthorizationError

import communication
import database.SqliteDB
import dispatch.SchedulingStrategy as SS
from blueprints import admin, auth, containers, machines, user
from communication import BaseServer, DockerController
from database import BaseDB, InfoCache
from dispatch import WaitQueue

os.chdir(os.path.dirname(__file__))

app = Quart(__name__)
app.register_blueprint(auth, url_prefix="/api/auth")
app.register_blueprint(admin, url_prefix="/api/admin")
app.register_blueprint(containers, url_prefix="/api/containers")
app.register_blueprint(machines, url_prefix="/api/machines")
app.register_blueprint(user, url_prefix="/api/user")

app.config['JWT_SECRECT_KEY'] = 'mycreditentials'
jwt = JWTManager(app)
cors(app)


def data_handler_gpus(info: dict, machine_id: int | str):
    """ deal with the gpu info sent by the machine

    Args:
        info (dict): the gpu info sent by the machine. e.g. {'0': {'type': 'Nvidia RTX 3060Ti', 'memory_total': 10240, 'memory_used': 2048, 'utilization': 0.96 }, ...}
        machine_id (int | str): the id of the machine
    """
    g.gpus_cache.update(machine_id, info)


def data_handler_image(data: dict, machine_id: int | str):
    if data['status'] == 'failed':
        g.error_logger(f'{machine_id} {data["opt"]} image:{data["image"]} failed, error info: {data["error"]}')
        g.logger(f'{machine_id} {data["opt"]} image:{data["image"]} failed')
        return
    g.logger(f'{machine_id} {data["opt"]} image:{data["image"]} success')
    if data['opt'] == 'pull':

        ...
    elif data['opt'] == 'remove':

        ...


def data_handler_container(data: dict, machine_id: int | str):
    user_id = data['user_id']
    massage_cache: InfoCache = g.massage_cache
    if massage_cache.contains(user_id):
        massage_queue: Queue = Queue()
    else:
        massage_queue: Queue = massage_cache.get(user_id)
    massage_queue.put({'type': 'container', 'opt': data['opt'], "status": data["status"]})
    massage_cache.update(user_id, massage_queue)

    db: BaseDB = g.db
    g.logger(f'{machine_id} {data["opt"]} container:{data["container"]} success')
    if data['opt'] == 'create':
        if data['status'] == 'failed':
            g.error_logger(f'{machine_id} {data["opt"]} container:{data["container"]} failed, error info: {data["error"]} create args: {json.dumps(data["original_data"])}')
            g.logger(f'{machine_id} {data["opt"]} container:{data["container"]} failed')
            return
        g.logger(f'{machine_id} {data["opt"]} container:{data["container"]} success')
        original_data = data['original_data']
        showname = original_data["name"].split('_')[-1][1:]
        portlist = [[int(k.split('/')[0]), v] for k, v in original_data["ports"].items()]
        image_id = db.get_image(search_key={"imagename": original_data["image"]}, return_key=['id'])[0]["id"]
        db.insert_container({"showname": showname, "containername": data["name"], "userid": user_id, "machineid": machine_id, "portlist": portlist, "image": image_id, "runing": False})
        return
    if data['status'] == 'failed':
        g.error_logger(f'{machine_id} {data["opt"]} container:{data["container"]} failed, error info: {data["error"]}')
        g.logger(f'{machine_id} {data["opt"]} container:{data["container"]} failed')
        return
    if data['opt'] == 'remove':
        db.delete_container(search_key={"containername": data["container"], "machineid": machine_id})
    elif data['opt'] in ['start', 'stop', 'restart']:
        db.update_container(search_key={"containername": data["container"], "machineid": machine_id}, update_key={"running": 'start' in data['opt']})


def data_handler_task(data: dict, machine_id: int | str):
    if data['status'] == 'failed' and data['opt'] == 'run':
        g.error_logger(f'{machine_id} {data["opt"]} task failed, error info: {data["error"]}')
        g.logger(f'{machine_id} {data["opt"]} task failed')
        g.wq.finish_task(machine_id, data['task_id'])
        return


# all functions get two parameters, the first is the data (json), the second is the machine_id
data_handler_funcs = {'gpus': data_handler_gpus, 'image': data_handler_image, 'container': data_handler_container, 'task': data_handler_task}
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
    containers = info['containers']
    for container in containers:
        db.update_container(search_key={'containername': container['name'], 'machineid': machine_id}, update_key={'running': container['running']})
    return {'machine_id': machine_id}


def run_finish_mail(task: dict):
    return "", ""


def run_handler(machine_id: int | str, task: dict) -> None:
    messenger: BaseServer = g.messenger
    task['opt'] = 'run'
    messenger.send({'type': 'task', 'data': task}, machine_id)
    if hasattr(g, 'mail'):
        user_id = task['user_id']
        user = g.db.get_user(search_key={'id': user_id}, return_key=[''])[0]
        if user['email'] != '':
            g.mail.append(user['email'], user['nickname'], *run_finish_mail(task))


def finish_handler(machine_id: int | str, task: dict) -> None:
    messenger: BaseServer = g.messenger
    task['opt'] = 'finish'
    messenger.send({'type': 'task', 'data': task}, machine_id)
    if hasattr(g, 'mail'):
        user_id = task['user_id']
        user = g.db.get_user(search_key={'id': user_id}, return_key=[''])[0]
        if user['email'] != '':
            g.mail.append(user['email'], user['nickname'], *run_finish_mail(task))


@app.before_serving
def init():
    with open('WebServerConfig.yaml') as f:
        configs = yaml.load(f, Loader=yaml.FullLoader)
    g.configs = configs
    if configs['Components']['Logger']['enable']:
        from database import Logger
        g.logger = Logger(**configs['Components']['Logger']['args'])
    else:
        g.logger = print
    if configs['Components']['ErrorLogger']['enable']:
        from database import Logger
        g.error_logger = Logger(**configs['Components']['ErrorLogger']['args'])
    else:
        g.error_logger = print

    g.repository = configs['Docker']['repository']
    DB_Class = getattr(database, configs['Database']['Class'])
    g.db: BaseDB = DB_Class(db_path=configs['Database']['db_path'])
    Scheduler_Class = getattr(SS, configs['Dispatch']['Class'])
    g.wq = WaitQueue(Scheduler_Class(**configs['Dispatch']['args']), run_handler, g.logger)
    g.wq.start()
    Messenger_Class = getattr(communication, configs['Components']['WebMessenger']['Class'])
    g.messenger: BaseServer = Messenger_Class(**configs['Components']['WebMessenger']['args'], data_handler=data_handler, connect_handler=connect_handler, disconnect_handler=disconnect_handler)
    g.messenger.start()  # start the messenger thread
    g.max_task_id = 0
    g.gpus_cache = InfoCache()
    g.massage_cache = InfoCache()
    g.docker = DockerController()
    if configs['Components']['Mail']['enable']:
        import communication.MailBox as MB
        Mail_Class = getattr(MB, configs['Components']['Mail']['Class'])
        g.mail = Mail_Class(**configs['Components']['Mail']['args'], logger=g.logger)


@app.before_request
async def is_jwt_valid():
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


async def sending():
    while True:
        await websocket.send(...)


async def receiving():
    while True:
        data = await websocket.receive()
        ...


@app.websocket('/ws')
async def ws():
    # if (
    #     websocket.authorization.username != USERNAME or
    #     websocket.authorization.password != PASSWORD
    # ):
    #     return 'Invalid password', 403  # or abort(403)
    g.clients[''] = websocket
    producer = asyncio.create_task(sending())
    consumer = asyncio.create_task(receiving())
    await asyncio.gather(producer, consumer)


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=10000, debug=False, threaded=True)
    # 目前有个bug,如果以debug模式运行,会导致套接字被定义两次,导致第二次无法绑定端口
