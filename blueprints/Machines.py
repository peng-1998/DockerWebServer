from quart import Blueprint, request, g, make_response, jsonify,current_app
from database import BaseDB
from dispatch import WaitQueue

machines = Blueprint('machines', __name__)



@machines.route('/info', methods=['GET'])
async def info():
    db: BaseDB = current_app.config['DB']
    machines = db.all_machine()
    return await make_response(jsonify(machines), 200)


@machines.route('/gpu_info/<machine_id>', methods=['GET'])
async def gpu_info(machine_id):
    gpus = current_app.config['gpus_cache'].get(machine_id)
    return await make_response(jsonify(gpus), 200)


@machines.route('/mytask/<machine_id>/<user_id>', methods=['GET'])
async def mytasks(machine_id: int | str, user_id: int | str):
    wq: WaitQueue = current_app.config['wq']
    tasks = wq.user_tasks(user_id, machine_id)
    return await make_response(jsonify(tasks), 200)


@machines.route('/request', methods=['POST'])
async def request_GPU():
    attrs = request.json
    wq: WaitQueue = current_app.config['wq']
    task = {'task_id': current_app.config['max_task_id'], **attrs}
    wq.new_task(task)
    current_app.config['max_task_id'] += 1
    return await make_response(jsonify(), 200)


@machines.route('/cancel', methods=['POST'])
async def cancel_GPU():
    attrs = request.json
    wq: WaitQueue = current_app.config['wq']
    task = wq.find_task(**attrs)
    if task['status'] == 'running':
        wq.finish_task(**attrs)
    else:
        wq.cancel_task(**attrs)
    return await make_response(jsonify(), 200)
