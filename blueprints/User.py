import bcrypt
from quart import Blueprint, jsonify, request, g, current_app,make_response
from database import BaseDB

user = Blueprint('user', __name__)

# @user.route('/', methods=['GET'])
# def index(): # 写ui的时候再定义或删除
#     ...
@user.route('/get_user/<account>', methods=['GET'])
async def get_user(account):
    db: BaseDB = current_app.config['DB']
    user = db.get_user({'account': account},return_key=['id', 'account', 'nickname', 'email', 'photo', 'phone'])
    return await make_response(jsonify(user), 200)

@user.route('/set_profile', methods=['POST'])
async def set_profile():
    attrs = request.json
    db: BaseDB = current_app.config['DB']
    db.update_user({'id': attrs['user_id']}, {attrs['field']: attrs['value']})
    return await make_response(jsonify(), 200)


@user.route('/set_photo/custom', methods=['POST'])
async def set_photo():
    user_id = request.headers['user_id']
    account = request.headers['account']
    photo   = request.files['photo']
    photo.save('./static/photo/custom/' + str(account) + '.' + photo.filename.split('0')[-1])
    db: BaseDB = current_app.config['DB']
    db.update_user({'id': user_id}, {'photo': 'custom/'+ account + '.' + photo.filename.split('0')[-1]})
    return await make_response(jsonify(), 200)

@user.route('/set_photo/default', methods=['POST'])
async def set_photo_default():
    attrs = request.json
    db: BaseDB = current_app.config['DB']
    db.update_user({'id': attrs['user_id']}, {'photo': 'default/'+ attrs['photo']})
    return await make_response(jsonify(), 200)

@user.route('/set_password', methods=['POST'])
async def set_password():
    attrs = request.json
    db: BaseDB = current_app.config['DB']
    user_id = attrs['user_id']
    new_password = attrs['new_password']
    salt = bcrypt.gensalt()
    salted_password = bcrypt.hashpw(new_password.encode(), salt)
    db.update_user({'id': user_id}, {'password': salted_password.decode(), 'salt': salt.decode()})
    return await make_response(jsonify(), 200)

