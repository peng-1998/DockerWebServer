from flask import Blueprint, request, g, make_response, jsonify
from communication import DockerController
from communication import BaseServer
from database import BaseDB

admin = Blueprint('admin', __name__)

# @admin.route('/', methods=['GET'])
# def index():
#     ...


@admin.route('/build_image', methods=['POST'])
def build_image():
    db: BaseDB = g.db
    attrs = request.json
    attrs['image'] = g.repository + '/' + attrs['image']

    docker: DockerController = g.docker
    res = docker.create_image(**attrs)
    if res:
        return make_response({'success': False, 'error': res}, 400)
    docker.push_image(attrs['image'])  # 该函数会阻塞到镜像推送完成 # 几乎不会失败
    if g.configs['Docker']['rmimageafterbuild']:
        docker.remove_image(attrs['image'])
    messenger: BaseServer = g.messenger
    messenger.send_all({'type': 'image', 'data': {'image': attrs['image'], 'opt': 'pull'}})  # 非阻塞，需要自己监听是否完成拉取
    db.insert_image({'imagename': attrs['image'], 'init_args': attrs['init_args'], 'description': attrs['description'], 'showname': attrs['showname']})
    return make_response({'success': True}, 200)


@admin.route('/delete_image', methods=['POST'])
def delete_image():
    attrs = request.json
    image_name = attrs['image_name']
    docker: DockerController = g.docker
    docker.remove_image(image_name)  # 该函数会阻塞到镜像删除完成，通常几秒钟
    messenger: BaseServer = g.messenger
    messenger.send_all({'type': 'image', 'data': {'image': image_name, 'opt': 'remove'}})  # 非阻塞，需要自己监听是否完成删除
    return make_response({'success': True}, 200)


@admin.route('/delete_webserver_container', methods=['POST'])
def delete_webserver_container():
    docker: DockerController = g.docker
    attrs = request.json
    container_names = attrs['container_names']
    res = docker.remove_container(container_names)
    response = {}
    for r, c in zip(res, container_names):
        if isinstance(r, bool):
            response[c] = r
        else:
            response[c] = False
    return make_response(jsonify(response), 200)
