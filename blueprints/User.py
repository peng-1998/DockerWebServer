from quart import Blueprint, jsonify, request, g, current_app,make_response
from database import BaseDB

user = Blueprint('user', __name__)

# @user.route('/', methods=['GET'])
# def index(): # 写ui的时候再定义或删除
#     ...
@user.route('/get_user/<account>', methods=['GET'])
async def get_user(account):
    db: BaseDB = current_app.config['DB']
    user = db.get_user({'account': account})
    return await make_response(jsonify(user), 200)

@user.route('/set_profile', methods=['POST'])
async def set_profile():
    attrs = request.json
    db: BaseDB = current_app.config['DB']
    db.update_user({'id': attrs['user_id']}, {attrs['field']: attrs['value']})
    return await make_response(jsonify(), 200)


@user.route('/set_photo', methods=['POST'])
async def set_photo():
    photo   = request.files['photo']
    user_id = request.json['user_id']
    photo.save('./static/photo/' + str(user_id) + '.' + photo.filename.split('0')[-1])
    db: BaseDB = current_app.config['DB']
    db.update_user({'id': user_id}, {'photo': str(user_id) + '.' + photo.filename.split('0')[-1]})
    return await make_response(jsonify(), 200)
