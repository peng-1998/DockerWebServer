from flask import Blueprint , request

machines = Blueprint('machines', __name__, url_prefix='/machines')

@machines.route('/', methods=['GET'])
def index():
    ...

@machines.route('/info', methods=['GET'])
def info():
    ...

@machines.route('/gpu_info/<machine_id>', methods=['GET'])
def gpu_info(machine_id):
    ...

@machines.route('/mytask/<machine_id>', methods=['GET'])
def mytask(machine_id):
    ...

@machines.route('/request', methods=['POST'])
def request_GPU(machine_id):
    attrs = request.json
    machine_id = attrs['machine_id']
    user_id = attrs['user_id']
    duration = attrs['duration']
    GPU = attrs['GPU']
    CMD = attrs['CMD']
    ...

@machines.route('/cancel', methods=['POST'])
def cancel_GPU(machine_id):
    attrs = request.json
    task_id = attrs['task_id']
    machine_id = attrs['machine_id']
    ...


