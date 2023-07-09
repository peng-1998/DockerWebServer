import asyncio
import os
import uuid

import yaml
from quart import Quart, Websocket, g, jsonify, request, websocket
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
from utils.AsyncStructure import AsyncDict
from communication.ServerManager import ServerManager
from eventhandlers.WebHandlers import run_handler, data_handler, connect_handler, disconnect_handler, finish_handler, ws_clinet_data_handlers

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


@app.before_serving
async def init():
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
    g.wq = WaitQueue(Scheduler_Class(**configs['Dispatch']['args']), run_handler, finish_handler, g.logger)
    g.wq.start()
    g.max_task_id = 0
    g.gpus_cache = AsyncDict()
    g.docker = DockerController()
    g.clients = AsyncDict()
    g.servers = ServerManager(data_handler=data_handler, connect_handler=connect_handler, disconnect_handler=disconnect_handler)
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


@app.websocket('/ws/client')
async def ws_client():
    # if (
    #     websocket.authorization.username != USERNAME or
    #     websocket.authorization.password != PASSWORD
    # ):
    #     return 'Invalid password', 403  # or abort(403)
    # 生成一个随机的uuid作为key
    key = str(uuid.uuid4())
    g.clients[key] = websocket
    try:
        while True:
            data = await websocket.receive_json()
            await ws_clinet_data_handlers[data['type']](data['data'], key)
    finally:
        del g.clients[uuid]


@app.websocket('/ws/server')
async def ws_server():
    machine_id = websocket.headers.get('machine_id', '')
    g.servers[machine_id] = websocket


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=10000, debug=False)
    # 目前有个bug,如果以debug模式运行,会导致套接字被定义两次,导致第二次无法绑定端口
