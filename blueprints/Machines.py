from flask import Blueprint, request, g, make_response, jsonify
from database import BaseDB
from dispatch import WaitQueue

machines = Blueprint('machines', __name__)

# @machines.route('/', methods=['GET'])
# def index():
#     ...


@machines.route('/info', methods=['GET'])
def info():
    db: BaseDB = g.db
    machines = db.get_machine({'id': '*'})
    return make_response(jsonify(machines), 200)


@machines.route('/gpu_info/<machine_id>', methods=['GET'])
def gpu_info(machine_id):
    gpus = g.gpus_cache.get(machine_id)
    return gpus


@machines.route('/mytask/<machine_id>/<user_id>', methods=['GET'])
def mytasks(machine_id: int | str, user_id: int | str):
    wq: WaitQueue = g.wq
    tasks = wq.user_tasks(user_id, machine_id)
    return make_response(jsonify(tasks), 200)


@machines.route('/request', methods=['POST'])
def request_GPU():
    attrs = request.json
    wq: WaitQueue = g.wq
    task = {'task_id': g.max_task_id, **attrs}
    wq.new_task(task)
    g.max_task_id += 1
    return make_response(jsonify(), 200)


@machines.route('/cancel', methods=['POST'])
def cancel_GPU():
    attrs = request.json
    wq: WaitQueue = g.wq
    task = wq.find_task(**attrs)
    if task['status'] == 'running':
        wq.finish_task(**attrs)
    else:
        wq.cancel_task(**attrs)
    return make_response(jsonify(), 200)
