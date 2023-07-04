from flask import Blueprint, request, g
from database import BaseDB

user = Blueprint('user', __name__, url_prefix='/user')


# @user.route('/', methods=['GET'])
# def index(): # 写ui的时候再定义或删除
#     ...


@user.route('/info', methods=['GET'])
def info(): # 写ui的时候再定义
    ...


@user.route('/set_profile', methods=['POST'])
def set_profile(field):
    attrs = request.json
    db: BaseDB = g.db
    db.update_user({'id': attrs['user_id']}, {attrs['field']: attrs['value']})
    return {'status': 'success'}


@user.route('/set_photo', methods=['POST'])
def set_photo():
    photo = request.files['photo']
    user_id = request.json['user_id']
    photo.save('./static/photo/' + str(user_id) + '.' + photo.filename.split('0')[-1])
    db: BaseDB = g.db
    db.update_user({'id': user_id}, {'photo': str(user_id) + '.' + photo.filename.split('0')[-1]})
    return {'status': 'success'}
