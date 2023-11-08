import bcrypt
from quart import Blueprint, jsonify, request, g, current_app, make_response, session
from database import BaseDB

user = Blueprint('user', __name__)


@user.route('/get_user', methods=['GET'])
async def get_user():
    account = session['account']
    db: BaseDB = current_app.config['DB']
    user = db.get_user({'account': account}, return_key=['id', 'account', 'nickname', 'email', 'photo', 'phone'])
    return await make_response(jsonify(user), 200)


@user.route('/set_profile', methods=['POST'])
async def set_profile():
    current_app.config['DB'].update_user({'id': session['user_id']}, request.json)
    return await make_response('', 204)


@user.route('/set_photo/custom', methods=['POST'])
async def set_photo():
    account = session['account']
    photo = request.files['photo']
    photo.save('./static/photo/custom/' + str(account) + '.' + photo.filename.split('0')[-1])
    db: BaseDB = current_app.config['DB']
    db.update_user({'account': account}, {'photo': 'custom/' + account + '.' + photo.filename.split('0')[-1]})
    return await make_response('', 204)


@user.route('/set_photo/default', methods=['POST'])
async def set_photo_default():
    account = session['account']
    attrs = request.json
    db: BaseDB = current_app.config['DB']
    db.update_user({'account': account}, {'photo': 'default/' + attrs['photo']})
    return await make_response('', 204)


@user.route('/set_password', methods=['POST'])
async def set_password():
    attrs = request.json
    db: BaseDB = current_app.config['DB']
    user_id = attrs['user_id']
    new_password = attrs['new_password']
    salt = bcrypt.gensalt()
    salted_password = bcrypt.hashpw(new_password.encode(), salt)
    db.update_user({'id': user_id}, {'password': salted_password.decode(), 'salt': salt.decode()})
    return await make_response('', 204)
