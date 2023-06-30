from flask import Blueprint , request


user = Blueprint('user', __name__, url_prefix='/user')

@user.route('/', methods=['GET'])
def index():
    ...

@user.route('/info', methods=['GET'])
def info():
    ...

@user.route('/set_email', methods=['POST'])
def set_email():
    email = request.json['email']
    ...

@user.route('/set_phone', methods=['POST'])
def set_phone():
    phone = request.json['phone']
    ...

@user.route('/set_password', methods=['POST'])
def set_password():
    password = request.json['password']
    ...

@user.route('/set_nickname', methods=['POST'])
def set_nickname():
    nickname = request.json['nickname']
    ...

@user.route('/set_photo', methods=['POST'])
def set_photo():
    photo = request.files['photo']
    photo.save('path/to/location')
    ...



