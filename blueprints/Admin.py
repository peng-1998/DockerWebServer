from quart import Blueprint, jsonify, make_response, request,current_app,session

from communication import BaseServer, DockerController
from database import BaseDB

admin = Blueprint('admin', __name__)

# @admin.before_request
# async def is_admin():
#     if 'is_ad'

# 转移到ws
@admin.route('/build_image', methods=['POST'])
async def build_image(): # 创建公共镜像
    db: BaseDB = current_app.config['DB']
    attrs = request.json
    attrs['image'] = current_app.config['repository'] + '/public/' + attrs['image']

    docker: DockerController = current_app.config['docker']
    res = docker.create_image(**attrs)
    if res:
        return await make_response({'success': False, 'error': res}, 400)
    docker.push_image(attrs['image'])  # 该函数会阻塞到镜像推送完成 # 几乎不会失败，因为仓库位于本地，但是如果仓库空间用完了就会失败
    if current_app.config['configs']['Docker']['rmimageafterbuild']:
        docker.remove_image(attrs['image']) # 失败概率趋近于0
    messenger: BaseServer = current_app.config['messenger']
    messenger.send_all({'type': 'image', 'data': {'image': attrs['image'], 'opt': 'pull'}})  # 非阻塞，需要自己监听是否完成拉取
    db.insert_image({'imagename': attrs['image'], 'init_args': attrs['init_args'], 'description': attrs['description'], 'showname': attrs['showname']})
    return await make_response({'success': True}, 200)

# 转移到ws
@admin.route('/delete_image', methods=['POST'])
async def delete_image():
    attrs = request.json
    image_name = attrs['image_name']
    docker: DockerController = current_app.config['docker']
    docker.remove_image(image_name)  # 该函数会阻塞到镜像删除完成，通常几秒钟
    messenger: BaseServer = current_app.config['messenger']
    messenger.send_all({'type': 'image', 'data': {'image': image_name, 'opt': 'remove'}})  # 非阻塞，需要自己监听是否完成删除
    return await make_response({'success': True}, 200)

# 转移到ws
@admin.route('/delete_webserver_container', methods=['POST'])
async def delete_webserver_container():
    docker: DockerController = current_app.config['docker']
    attrs           = request.json
    container_names = attrs['container_names']
    res             = docker.remove_container(container_names)
    response = {}
    for r, c in zip(res, container_names):
        if isinstance(r, bool):
            response[c] = r
        else:
            response[c] = False
    return await make_response(jsonify(response), 200)


@admin.route('/alluser', methods=['GET'])
async def alluser():
    db: BaseDB = current_app.config['DB']
    users = db.all_user(return_key=['account', 'nickname', 'email', 'phone'])
    return await make_response(jsonify(users), 200)


@admin.route('/allimage', methods=['GET'])
async def allimage():
    db: BaseDB = current_app.config['DB']
    images = db.all_image(return_key=['imagename', 'showname', 'description'])
    return await make_response(jsonify(images), 200)


@admin.route('/allcontainer/<machine_id>', methods=['GET'])
async def allcontainer(machine_id):
    db: BaseDB = current_app.config['DB']
    containers = db.get_container(search_key={'machine_id': machine_id}, return_key=['showname', 'userid', 'imageid', 'running','portlist','containername'])
    for c in containers:
        c['username'] = db.get_user(search_key={'id': c['userid']}, return_key=['account'])[0]['account']
        c['imagename'] = db.get_image(search_key={'id': c['imageid']}, return_key=['imagename'])[0]['imagename']
    return await make_response(jsonify(containers), 200)


