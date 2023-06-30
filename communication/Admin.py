from flask import Blueprint , request


admin = Blueprint('admin', __name__, url_prefix='/admin')

@admin.route('/', methods=['GET'])
def index():
    ...


@admin.route('/build_image', methods=['POST'])
def build_image():
    attrs = request.json
    dockerfile = attrs['dockerfile']
    image_name = attrs['image_name']
    tag = attrs['tag']
    showname = attrs['showname']
    creat_args = attrs['creat_args']
    description = attrs['description']
    ...

@admin.route('/delete_image', methods=['POST'])
def delete_image():
    attrs = request.json
    image_id = attrs['image_id']
    ...
