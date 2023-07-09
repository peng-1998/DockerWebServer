
import json
import random
import string
from quart import g
from communication.BaseMessenger import BaseServer
from database import BaseDB

from utils.AsyncStructure import AsyncDict, AsyncQueue

async def data_handler_gpus(info: dict, machine_id: int | str):
    """ deal with the gpu info sent by the machine

    Args:
        info (dict): the gpu info sent by the machine. e.g. {'0': {'type': 'Nvidia RTX 3060Ti', 'memory_total': 10240, 'memory_used': 2048, 'utilization': 0.96 }, ...}
        machine_id (int | str): the id of the machine
    """
    await g.gpus_cache.__setitem__(machine_id, info)


async def data_handler_image(data: dict, machine_id: int | str):
    if data['status'] == 'failed':
        g.error_logger(f'{machine_id} {data["opt"]} image:{data["image"]} failed, error info: {data["error"]}')
        g.logger(f'{machine_id} {data["opt"]} image:{data["image"]} failed')
        return
    g.logger(f'{machine_id} {data["opt"]} image:{data["image"]} success')
    if data['opt'] == 'pull':

        ...
    elif data['opt'] == 'remove':

        ...


async def data_handler_container(data: dict, machine_id: int | str):
    user_id = data['user_id']
    await g.clients[data['uuid']].send_json({'type': 'container', 'opt': data['opt'], "status": data["status"]})
    db: BaseDB = g.db

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


async def data_handler_task(data: dict, machine_id: int | str):
    if data['status'] == 'failed' and data['opt'] == 'run':
        g.error_logger(f'{machine_id} {data["opt"]} task failed, error info: {data["error"]}')
        g.logger(f'{machine_id} {data["opt"]} task failed')
        g.wq.finish_task(machine_id, data['task_id'])
        return


# all functions get two parameters, the first is the data (json), the second is the machine_id
data_handler_funcs = {'gpus': data_handler_gpus, 'image': data_handler_image, 'container': data_handler_container, 'task': data_handler_task}
# data: {'type': str, 'data': dict}




async def connect_handler(info: dict, ip: str) -> dict:
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

async def disconnect_handler(machine_id: int | str):
    g.wq.remove_machine(machine_id)

async def data_handler(data: dict, machine_id: int | str):
    """ deal with the data sent by the machine

    Args:
        data (dict): the data sent by the machine. e.g. {'type': 'gpus', 'data': {'0': {'type': 'Nvidia RTX 3060Ti', 'memory_total': 10240, 'memory_used': 2048, 'utilization': 0.96 }, ...}}
        machine_id (int | str): the id of the machine
    """
    await data_handler_funcs[data['type']](data['data'], machine_id)


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


async def ws_handler_container(data: dict,uuid: str):
    db: BaseDB = g.db
    if data['opt'] == 'create':
        user_id = data['user_id']
        account = data['account']
        image_id = data['image_id']
        image = db.get_image({'id': image_id})
        machine_id = data['machine_id']
        ports = data['ports']
        chars = string.ascii_letters + string.digits
        random_string = ''.join(random.choice(chars) for i in range(10))
        msg = {
            'type': 'container',
            'data': {
                'opt': 'create',
                'user_id': user_id,
                'uuid': uuid,
                'create_args': {
                    'name': f'u{account}_c{random_string}',
                    'image': image['imagename'],
                    'ports': ports,
                    'hostname': account,
                    **image['init_args'],
                }
            },
        }
        g.messenger.send(msg, machine_id)
    elif data['opt'] in ['start','stop','restart','remove']:
        msg = {
            'type': 'container',
            'data': {
                'opt': data['opt'],
                'uuid': uuid,
                'container_name': data['container_name'],
                'user_id': user_id,
            }
        }
        g.messenger.send(msg, machine_id)
    
ws_handlers = {
    'container': ws_handler_container,
}