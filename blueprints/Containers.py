import random
import string
from queue import Queue

from flask import Blueprint, g, jsonify, make_response, request

from database import BaseDB

containers = Blueprint('containers', __name__)

# @containers.route('/', methods=['GET'])
# def index():
#     ...


@containers.route('/mycontainer/<user_id>', methods=['GET'])
def mycontainer(user_id):
    db: BaseDB = g.db
    containers = db.get_container({'userid': user_id}, return_key=['showname', 'portlist', 'containername', 'running'])
    return make_response(jsonify(containers), 200)


@containers.route('/create', methods=['POST'])
def create_container():
    db: BaseDB = g.db
    attrs = request.json
    user_id = attrs['user_id']
    account = attrs['account']
    image_id = attrs['image_id']
    image = db.get_image({'id': image_id})
    if len(image) == 0:
        return make_response(jsonify(), 404)  # 没有找到镜像，讲道理，这个错误真的会发生吗？
    image = image[0]
    machine_id = attrs['machine_id']
    ports = attrs['ports']
    chars = string.ascii_letters + string.digits
    random_string = ''.join(random.choice(chars) for i in range(10))
    msg = {
        'type': 'container',
        'data': {
            'opt': 'create',
            'user_id': user_id,
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
    g.messenger.send(msg, machine_id)
    msg_queue: Queue = g.massage_cache.get(user_id)
    while True:
        msg = msg_queue.get()
        if msg['type'] == 'container' and msg['opt'] == 'create':
            break
        else:
            msg_queue.put(msg)
    if msg['status'] == 'success':
        return make_response(jsonify(), 200)
    else:
        return make_response(jsonify(), 400)


@containers.route('/operate', methods=['POST'])
def container_operate():
    attrs = request.json
    container_name = attrs['container_name']
    user_id = attrs['user_id']
    machine_id = attrs['machine_id']
    opt = attrs['opt']  # start, stop, restart, remove
    msg = {
        'type': 'container',
        'data': {
            'opt': opt,
            'container_name': container_name,
            'user_id': user_id,
        }
    }
    g.messenger.send(msg, machine_id)
    msg_queue: Queue = g.massage_cache.get(user_id)
    while True:
        msg = msg_queue.get()
        if msg['type'] == 'container' and msg['opt'] == opt:
            break
        else:
            msg_queue.put(msg)
    if msg['status'] == 'success':
        return make_response(jsonify(), 200)
    else:
        return make_response(jsonify(), 400)

@containers.route('/commitimage', methods=['POST'])
def commit_image():
    attrs = request.json
    container_name = attrs['container_name']
    account = attrs['account']
    machine_id = attrs['machine_id']
    image_name = attrs['image_name']
    father_image_id = attrs['father_image_id']
    msg = {
        'type': 'container',
        'data': {
            'opt': 'commit',
            'container_name': container_name,
            'account': account,
            'image_name': image_name,
            'father_image_id': father_image_id,
        }
    }
    g.messenger.send(msg, machine_id)
    msg_queue: Queue = g.massage_cache.get(account) 
    while True:
        msg = msg_queue.get() # 这里会阻塞，直到收到消息
        if msg['type'] == 'container' and msg['opt'] == 'commit':
            break
        else:
            msg_queue.put(msg)
    if msg['status'] == 'success':
        return make_response(jsonify(), 200)
    else:
        return make_response(jsonify(), 400)



