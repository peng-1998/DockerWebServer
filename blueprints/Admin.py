from flask import Blueprint, request, g, make_response
from communication import DockerController
from communication import BaseServer
from database import BaseDB

admin = Blueprint('admin', __name__)

# @admin.route('/', methods=['GET'])
# def index():
#     ...


@admin.route('/build_image', methods=['POST'])
def build_image():
    attrs = request.json
    attrs['image'] = g.repository + '/' + attrs['image']
    docker: DockerController = g.docker
    try:
        docker.create_image(**attrs)  # 该函数会阻塞到镜像构建完成
    except Exception as e:
        print(e)
        return make_response({'success': False}, 400)
    messenger: BaseServer = g.messenger
    messenger.send_all({'type': 'image', 'data': {'image': attrs['image'], 'opt': 'pull'}})  # 非阻塞，需要自己监听是否完成拉取
    return make_response({'success': True}, 200)


@admin.route('/delete_image', methods=['POST'])
def delete_image():
    db: BaseDB = g.db
    attrs = request.json
    image_id = attrs['image_id']
    docker: DockerController = g.docker
    docker.remove_image(image_id)  # 该函数会阻塞到镜像删除完成，通常几秒钟
    messenger: BaseServer = g.messenger
    image = db.get_image({'id': image_id}, ['imagename'])[0]['imagename']
    messenger.send_all({'type': 'image', 'data': {'image': image, 'opt': 'remove'}})  # 非阻塞，需要自己监听是否完成删除
