import importlib
import json
import socket
from database import BaseDB
from dispatch import WaitQueue
import bcrypt
import yaml
from flask import Flask, g, jsonify, make_response, request
from flask_cors import CORS
from werkzeug.security import check_password_hash, generate_password_hash

app = Flask(__name__)
CORS(app)


def data_handler(data_j: dict):

    ...


def connect_handler_TCP(client_socket: socket.socket, client_address: tuple):
    data = client_socket.recv(1024)
    data = json.loads(data.decode())
    machine_id = data['machine_id']
    gpus = data['gpus']
    machines = g.db.get_machine(search_key={'id': machine_id})
    info = {'gpus': gpus}
    if len(machines) == 0:
        g.db.insert_machine(machine={'id': machine_id, 'ip': client_address[0], 'machine_info': info})
    else:
        g.db.update_machine(search_key={'id': machine_id}, update_key={'machine_info': info, 'ip': client_address[0]})
    g.wq.new_machine(machine_id, {i: True for i in range(len(gpus))})
    ...


def run_handler(machine_id: int | str, task: dict):
    ...


def init():
    configs = yaml.load(open('config.yaml'), Loader=yaml.FullLoader)
    DB_Class = importlib.import_module('database.' + configs['Database']['Class'])
    g.db: BaseDB = DB_Class(db_path=configs['Database']['db_path'])
    Scheduler_Class = importlib.import_module('dispatch.SchedulingStrategy.' + configs['Dispatch']['Class'])
    g.wq = WaitQueue(Scheduler_Class(**configs['Dispatch']['args']), run_handler)

    # Messenger_Class = importlib.import_module('components.' + configs['Components']['Messenger']['Class'])
    # g.messenger = Messenger_Class(**configs['Components']['Messenger']['args'])

    if configs['Components']['Mail']['enable']:
        Mail_Class = importlib.import_module('components.MailBox.' + configs['Components']['Mail']['Class'])
        g.mail = Mail_Class(**configs['Components']['Mail']['args'])


@app.teardown_appcontext
def close_db(error):
    if 'db' in g:
        g.db.close()


@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"


@app.route('/api/login', methods=('GET', 'POST'))
def login():
    data = request.get_json()
    user_name = data.get('username')
    print(user_name)
    database = get_db()
    user_info_list = database.get_user(search_key={'username': user_name}, return_key=['password', 'salt'])
    if user_info_list:
        saved_password, salt = user_info_list[0]
        password = bcrypt.hashpw(data.get('password').encode(), salt.encode())
        if saved_password == password.decode():
            return make_response(jsonify(success=True, message="Login Succeed"), 200)
        else:
            return make_response(jsonify(success=False, message="Wrong Password"), 401)
    else:
        return make_response(jsonify(success=False, message="User Not Found"), 404)


@app.route('/api/register', methods=['POST'])
def register():
    database = get_db()
    data = request.get_json()
    username = data.get('username')
    hashed_password = data.get('password')
    is_user_name_exists = database.get_user(search_key={'username': username})
    if is_user_name_exists:
        return make_response(jsonify(success=False, message="用户已存在"), 409)
    else:
        salt = bcrypt.gensalt()
        hashed_password = bcrypt.hashpw(hashed_password.encode(), salt)
        database.insert_user(user={'username': username, 'password': hashed_password.decode(), 'salt': salt.decode()})
        return make_response(jsonify(success=True, message="注册成功"), 200)


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=9998, debug=True, threaded=True)