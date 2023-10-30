import bcrypt
from quart import Blueprint, g, jsonify, make_response, request, current_app
from quart_cors import cors
from quart_jwt_extended import create_access_token

from database import BaseDB

auth = Blueprint('auth', __name__)
cors(auth)


@auth.route('/login', methods=(['POST']))
async def login():
    database: BaseDB = current_app.config['DB']
    data           = await request.get_json()
    account       = data.get('account')
    user_info_list = database.get_user(search_key={'account': account}, return_key=['password', 'salt'])
    if user_info_list:
        saved_password, salt = user_info_list[0]
        password = bcrypt.hashpw(data.get('password').encode(), salt.encode())
        if saved_password == password.decode():
            access_token = create_access_token(identity=account)
            return await make_response(jsonify(access_token=access_token), 200)
        else:
            return await make_response(jsonify(), 401)
    else:
        return await make_response(jsonify(), 404)


@auth.route('/register', methods=['POST'])
async def register():
    database: BaseDB = current_app.config['DB']
    data     = await request.get_json()
    account  = data.get('account')
    password = data.get('password')
    is_user_name_exists = database.get_user(search_key={'account': account})
    if len(is_user_name_exists) != 0:
        return await make_response(jsonify(), 409)
    else:
        salt            = bcrypt.gensalt()
        hashed_password = bcrypt.hashpw(password.encode(), salt)
        database.insert_user(user={'account': account, 'password': hashed_password.decode(), 'salt': salt.decode()})
        return await make_response(jsonify(), 200)
