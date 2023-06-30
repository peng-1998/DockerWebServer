from typing import Callable, OrderedDict
from readerwriterlock import rwlock
'''
task format:
{
    "task_id": int,
    "machine_id": int,
    "user_id": int,
    "duration": int,
    "GPU": list | int, # GPU number or list of GPU numbers
    "CMD": dict{
        "container_name": str,
        "command": str,
    }
}
source format:
{
    # GPU_number: bool,
    1: True,
    2: True,
}
'''



class WaitQueue:

    def __init__(self, scheduling_strategy: Callable, run_handler: Callable, logger: Callable = print):
        self.queues = {}
        self.scheduling_strategy = scheduling_strategy
        self.logger = logger
        self.run_handler = run_handler
        self.user_indices = {}
        self.queue_rw_lock = rwlock.RWLockWrite()
        self.indices_rw_lock = rwlock.RWLockWrite()

    def new_machine(self, machine_id: int | str, source: dict) -> None:
        with self.queue_rw_lock.gen_wlock():
            self.queues[machine_id] = {'wait_queue': OrderedDict(), 'running_set': {}, 'on_line': True, 'source': source}
        self.logger(f'create wait queue for machine {machine_id}')

    def new_task(self, machine_id: int | str, task: dict) -> dict | None:
        with self.queue_rw_lock.gen_wlock():
            self.queues[machine_id]['wait_queue'][task['task_id']] = task
        self.logger(f'user {task["user_id"]} add task {task["task_id"]} to machine {machine_id} wait queue')
        user = task['user_id']
        if user not in self.user_indices:
            self.user_indices[user] = {}
        if machine_id not in self.user_indices[user]:
            self.user_indices[user][machine_id] = []
        with self.indices_rw_lock.gen_wlock():
            self.user_indices[user][machine_id].append(task['task_id'])
        return self.try_allot(machine_id)

    def try_allot(self, machine_id: int | str) -> dict | None:
        with self.queue_rw_lock.gen_rlock():
            self.queues[machine_id].freeze()
            results: list[dict] = self.scheduling_strategy(self.queues[machine_id])
            self.queues[machine_id].unfreeze()
        if not results:
            return
        with self.queue_rw_lock.gen_wlock():
            for result in  results:
                task = self.queues[machine_id]['wait_queue'][result['task_id']]
                if isinstance(task['GPU'], int):
                    task['GPU'] = result['GPU']
                self.queues[machine_id]['running_set'][result['task_id']] = task
                del self.queues[machine_id]['wait_queue'][result['task_id']]
                for g in result['GPU']:
                    self.queues[machine_id]['source'][g] = False
        self.logger(f'user {task["user_id"]} start task {task["task_id"]} on machine {machine_id}')
        self.run_handler(machine_id, task.copy())

    def finish_task(self, machine_id: int | str, task_id: int) -> dict | None:
        with self.queue_rw_lock.gen_rlock():
            if task_id not in self.queues[machine_id]['running_set']:
                self.logger(f'error: task {task_id} not in machine {machine_id} running set')
                return
        with self.queue_rw_lock.gen_wlock():
            task = self.queues[machine_id]['running_set'][task_id]
            del self.queues[machine_id]['running_set'][task_id]
            for g in task['GPU']:
                self.queues[machine_id]['source'][g] = True
        with self.indices_rw_lock.gen_wlock():
            self.user_indices[task['user_id']][machine_id].remove(task_id)
        self.logger(f'user {task["user_id"]} finish task {task["task_id"]} on machine {machine_id}')
        self.try_allot(machine_id)
        return task

    def cancel_task(self, machine_id: int | str, task_id: int) -> dict | None:
        with self.queue_rw_lock.gen_rlock():
            if task_id not in self.queues[machine_id]['wait_queue']:
                self.logger(f'error: task {task_id} not in machine {machine_id} running set')
                return
        with self.queue_rw_lock.gen_wlock():
            task = self.queues[machine_id]['wait_queue'][task_id]
            del self.queues[machine_id]['wait_queue'][task_id]
        with self.indices_rw_lock.gen_wlock():
            self.user_indices[task['user_id']][machine_id].remove(task_id)
        self.logger(f'user {task["user_id"]} cancel task {task["task_id"]} on machine {machine_id}')
        return task

    def find_task(self, machine_id: int | str, task_id: int) -> dict | None:
        with self.queue_rw_lock.gen_rlock():
            if task_id in self.queues[machine_id]['wait_queue']:
                res = self.queues[machine_id]['wait_queue'][task_id].copy()
                res['status'] = 'wait'
                return res
            if task_id in self.queues[machine_id]['running_set']:
                res = self.queues[machine_id]['running_set'][task_id].copy()
                res['status'] = 'running'
                return res
        return None

    def set_machine_offline(self, machine_id: int | str, online: bool) -> None:
        with self.queue_rw_lock.gen_wlock():
            self.queues[machine_id]['on_line'] = online

    def machine_tasks(self, machine_id: int | str) -> dict:
        with self.queue_rw_lock.gen_rlock():
            res = []
            machine = self.queues[machine_id]
            for _, task in machine['wait_queue']:
                res.append(task.copy())
                res[-1]['status'] = 'wait'
            for _, task in machine['running_set']:
                res.append(task.copy())
                res[-1]['status'] = 'running'
            return res
