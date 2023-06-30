from flask import Blueprint , request

containers = Blueprint('containers', __name__, url_prefix='/containers')


@containers.route('/', methods=['GET'])
def index():
    ...

@containers.route('/mycontainer/<user_id>', methods=['GET'])
def mycontainer(user_id):
    ...

@containers.route('/create', methods=['POST'])
def create_container():
    attrs = request.json
    user_id = attrs['user_id']
    image_id = attrs['image_id']
    ports = attrs['ports']
    ...

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



