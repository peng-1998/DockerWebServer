from flask import Blueprint, request, g, make_response, jsonify
import random
import string
from database import BaseDB

containers = Blueprint('containers', __name__, url_prefix='/containers')

# @containers.route('/', methods=['GET'])
# def index():
#     ...


@containers.route('/mycontainer/<user_id>', methods=['GET'])
def mycontainer(user_id):
    ...


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
            'name': f'u{user_id}_c{random_string}',
            'image': image['imagename'] + ':' + image['tag'],
            'ports': ports,
            'hostname': account,
            **image['init_args'],
        },
    }
    g.messenger.send(msg, machine_id)
    


@containers.route('/delete', methods=['POST'])
def delete_container():
    attrs = request.json
    container_id = attrs['container_id']
    ...


@containers.route('/start', methods=['POST'])
def start_container():
    attrs = request.json
    container_id = attrs['container_id']
    ...


@containers.route('/stop', methods=['POST'])
def stop_container():
    attrs = request.json
    container_id = attrs['container_id']
    ...


@containers.route('/restart', methods=['POST'])
def restart_container():
    attrs = request.json
    container_id = attrs['container_id']
    ...


@containers.route('/container/<container_id>', methods=['GET'])
def container(container_id):
    ...
