from flask import Blueprint, request, g
from docker import DockerController
from communication import BaseServer

admin = Blueprint('admin', __name__, url_prefix='/admin')


@admin.route('/', methods=['GET'])
def index():
    ...


@admin.route('/build_image', methods=['POST'])
def build_image():
    attrs = request.json
    attrs['image'] = g.repository + '/' + attrs['image']
    docker: DockerController = g.docker
    docker.create_image(**attrs)
    messenger: BaseServer = g.messenger
    messenger.send_all({'type': 'image', 'data': {'image': attrs['image'], 'tag': attrs['tag'], 'opt': 'pull'}})


@admin.route('/delete_image', methods=['POST'])
def delete_image():
    attrs = request.json
    image_id = attrs['image_id']
    docker: DockerController = g.docker
    docker.remove_image(image_id)
    messenger: BaseServer = g.messenger
    messenger.send_all({'type': 'image', 'data': {'image': image_id, 'opt': 'remove'}})