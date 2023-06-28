from typing import Callable, OrderedDict


'''
task format:
{
    "task_id": int,
    "machine_id": int,
    "user_id": int,
    "duration": int,
    "GPU": list,
    "CMD": dict{
        "container_name": str,
        "command": str,
    }
'''

class WaitQueue:

    def __init__(self, scheduling_strategy: Callable, logger: Callable = print):
        self.queues = {}
        self.scheduling_strategy = scheduling_strategy
        self.logger = logger

    def new_machine(self, machine_id: int | str, source: dict) -> None:
        self.queues[machine_id] = {'wait_queue': OrderedDict(), 'running_set': {}, 'on_line': True, 'source': source}
        self.logger(f'create wait queue for machine {machine_id}')

    def new_task(self, machine_id: int | str, task: dict) -> dict | None:
        self.queues[machine_id]['wait_queue'][task['task_id']] = task
        self.logger(f'user {task["user_id"]} add task {task["task_id"]} to machine {machine_id} wait queue')
        return self.try_allot(machine_id)

    def try_allot(self, machine_id: int | str) -> dict | None:
        result = self.scheduling_strategy(machine_id, self.queues[machine_id])

        ...
