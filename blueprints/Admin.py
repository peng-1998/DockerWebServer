from flask import Blueprint, g, jsonify, make_response, request

from communication import BaseServer, DockerController
from database import BaseDB

admin = Blueprint('admin', __name__)

# @admin.route('/', methods=['GET'])
# def index():
#     ...


@admin.route('/build_image', methods=['POST'])
def build_image(): # 创建公共镜像
    db: BaseDB = g.db
    attrs = request.json
    attrs['image'] = g.repository + '/public/' + attrs['image']

    docker: DockerController = g.docker
    res = docker.create_image(**attrs)
    if res:
        return make_response({'success': False, 'error': res}, 400)
    docker.push_image(attrs['image'])  # 该函数会阻塞到镜像推送完成 # 几乎不会失败，因为仓库位于本地，但是如果仓库空间用完了就会失败
    if g.configs['Docker']['rmimageafterbuild']:
        docker.remove_image(attrs['image']) # 失败概率趋近于0
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


