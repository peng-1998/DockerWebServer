import datetime
from typing import TypedDict


GPURequest = TypedDict('GPURequest', id=int, duration=int, user=str, cmd=str, container=str, start_time=datetime.datetime, end_time=datetime.datetime)


class GPUQueueManager:

    def __init__(self, gpucount: int = 1) -> None:
        self.gpucount = gpucount
        self.gpu_wait_queues = [[] for _ in range(gpucount)]

    def new_item(self, id: int, gpuid: int, user: str, duration: int, container: str = None, cmd: str = None) -> GPURequest:
        current_queue = self.gpu_wait_queues[gpuid]
        if len(current_queue) == 0:
            start_time = datetime.datetime.now()
        else:
            start_time = current_queue[-1]['end_time']
        end_time = start_time + datetime.timedelta(hours=duration)
        item = GPURequest(id=id, duration=duration, user=user, container=container, cmd=cmd, start_time=start_time, end_time=end_time)
        current_queue.append(item)
        return item

    def current_item(self):
        for queue in self.gpu_wait_queues:
            if len(queue) == 0:
                yield None
            else:
                yield queue[0]

    def stopearly(self, gpuid: int):
        self.gpu_wait_queues[gpuid].pop(0)
        if len(self.gpu_wait_queues[gpuid])!=0:
            self.update_queue()
            return self.gpu_wait_queues[gpuid][0]
        return None

    def update_queue(self):
        for queue in self.gpu_wait_queues:
            if len(queue) != 0:
                q1:GPURequest = queue[0]
                if q1['start_time'] > datetime.datetime.now():
                    q1['start_time'] = datetime.datetime.now()
                    q1['end_time'] = q1['start_time'] + datetime.timedelta(hours=q1['duration'])
                pre = q1
                for rq in queue[1:]:
                    rq['start_time'] = pre['end_time']
                    rq['end_time'] = rq['start_time'] + datetime.timedelta(hours=rq['duration'])
                    pre = rq


