import bcrypt
from flask import Blueprint, g, jsonify, make_response, request
from flask_cors import CORS
from flask_jwt_extended import create_access_token

from database import BaseDB

auth = Blueprint('auth', __name__)
CORS(auth)

@auth.route('/login', methods=(['POST']))
def login():
    database: BaseDB = g.db
    data = request.get_json()
    username = data.get('username')
    user_info_list = database.get_user(search_key={'username': username}, return_key=['password', 'salt'])
    if user_info_list:
        saved_password, salt = user_info_list[0]
        password = bcrypt.hashpw(data.get('password').encode(), salt.encode())
        if saved_password == password.decode():
            access_token = create_access_token(identity=username)
            return make_response(jsonify(success=True, message="Login Succeed", access_token=access_token), 200)
        else:
            return make_response(jsonify(success=False, message="Wrong Password"), 401)
    else:
        return make_response(jsonify(success=False, message="User Not Found"), 404)


@auth.route('/register', methods=['POST'])
def register():
    database: BaseDB = g.db
    data = request.get_json()
    username = data.get('username')
    password = data.get('password')
    is_user_name_exists = database.get_user(search_key={'username': username})
    if is_user_name_exists:
        return make_response(jsonify(success=False, message="User Alreay Exists"), 409)
    else:
        salt = bcrypt.gensalt()
        hashed_password = bcrypt.hashpw(password.encode(), salt)
        database.insert_user(user={'username': username, 'password': hashed_password.decode(), 'salt': salt.decode()})
        return make_response(jsonify(success=True, message="Register Succeed"), 200)
