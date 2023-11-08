import bcrypt
from quart import Blueprint, g, jsonify, make_response, request, current_app, session
# from quart_cors import cors
# from quart_jwt_extended import create_access_token

from database import BaseDB

auth = Blueprint('auth', __name__)
# cors(auth)


@auth.route('/login', methods=(['POST']))
async def login():
    database: BaseDB = current_app.config['DB']
    data = await request.get_json()
    account = data.get('account')
    user_info_list = database.get_user(search_key={'account': account}, return_key=['password', 'salt'])
    if user_info_list:
        saved_password, salt = user_info_list[0]
        password = bcrypt.hashpw(data.get('password').encode(), salt.encode())
        if saved_password == password.decode():
            # access_token = create_access_token(identity=account)
            session['account'] = account
            session['user_id'] = database.get_user(search_key={'account': account}, return_key=['id'])[0][0]
            session['logged_in'] = True
            current_app.logger.info(f'login account {account} success')
            # return await make_response(jsonify(access_token=access_token), 200)
            return await make_response('', 204)
        else:
            session['logged_in'] = False
            current_app.logger.info(f'login account {account} failed, wrong password')
            return await make_response('', 401)
    else:
        session['logged_in'] = False
        current_app.logger.info(f'login account {account} failed, account not exists')
        return await make_response('', 404)


@auth.route('/register', methods=['POST'])
async def register():
    database: BaseDB = current_app.config['DB']
    data = await request.get_json()
    account, password = data.get('account'), data.get('password')
    is_user_name_exists = database.get_user(search_key={'account': account}, return_key=['account'])
    if len(is_user_name_exists) != 0:
        current_app.logger.info(f'register account {account} failed, account already exists')
        return await make_response('', 409)
    else:
        hashed_password = bcrypt.hashpw(password.encode(), salt := bcrypt.gensalt())
        database.insert_user(user={'account': account, 'password': hashed_password.decode(), 'salt': salt.decode()})
        current_app.logger.info(f'register account {account} success')
        return await make_response('', 204)
