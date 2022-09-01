import datetime
from typing import List, TypedDict

GPURequest = TypedDict('GPURequest', id=int, duration=int, user=str, cmd=str, container=str, start_time=datetime.datetime, end_time=datetime.datetime, postponement=int)


class GPUQueueManager:

    def __init__(self, gpucount: int = 1) -> None:
        self.gpucount = gpucount
        self.gpu_wait_queues = [[] for _ in range(gpucount)]

    def new_item(self, id: int, gpuid: int, user: str, duration: int, container: str = None, cmd: str = None) -> GPURequest:
        current_queue = self.gpu_wait_queues[gpuid]
        if len(current_queue) == 0:
            start_time = datetime.datetime.now()  # 如果队列为空，直接开始
        else:
            start_time = current_queue[-1]['end_time']  # 如果队列不为空，等待队列最后一个结束
        end_time = start_time + datetime.timedelta(hours=duration)  # 结束时间
        item = GPURequest(id=id, duration=duration, user=user, container=container, cmd=cmd, start_time=start_time, end_time=end_time, postponement=0)
        current_queue.append(item)
        return item

    def current_item(self) -> GPURequest | None:
        for queue in self.gpu_wait_queues:
            if len(queue) == 0:
                yield None
            else:
                yield queue[0]

    def stopearly(self, gpuid: int):
        self.gpu_wait_queues[gpuid].pop(0)
        if len(self.gpu_wait_queues[gpuid]) != 0:
            self.update_queue()
            return self.gpu_wait_queues[gpuid][0]
        return None

    def stop(self, gpuid: int):
        self.gpu_wait_queues[gpuid].pop(0)
        if len(self.gpu_wait_queues[gpuid]) != 0:
            return self.gpu_wait_queues[gpuid][0]
        return None

    def update_queue(self) -> None:
        for queue in self.gpu_wait_queues:
            if len(queue) != 0:
                q1: GPURequest = queue[0]
                if q1['start_time'] > datetime.datetime.now():  # 如果当前时间小于开始时间，则第一个申请的开始时间设为现在
                    q1['start_time'] = datetime.datetime.now()
                    q1['end_time'] = q1['start_time'] + datetime.timedelta(hours=q1['duration'])
                pre = q1
                for rq in queue[1:]:
                    rq['start_time'] = pre['end_time']
                    rq['end_time'] = rq['start_time'] + datetime.timedelta(hours=rq['duration'])
                    pre = rq

    def now_stop(self, gpuid: int):
        """某一申请结束时的逻辑,执行时机:1 申请时间到达 2 从容器当中发出的通知 3 web端发出的通知

        Args:
            gpuid (int): GPU id
        """

        def find_frist_item(queue: List, need_cmd: bool = True) -> int:
            for idx, item in enumerate(queue):
                if (need_cmd and item['cmd'] is not None and item['container'] is not None) or (not need_cmd and item['cmd'] is None):
                    return idx
            return -1

        self.gpu_wait_queues[gpuid].pop(0)  # 当前执行的申请出队列
        if len(self.gpu_wait_queues[gpuid]) == 0:  # 没有需要执行的申请
            return None

        self.update_queue()  # 更新队列时间

        now_time_hour = datetime.datetime.now().hour
        if now_time_hour < 8 or now_time_hour > 22:  # 晚上10点到早上8点不执行无指令的申请
            q1: GPURequest = self.gpu_wait_queues[gpuid][0]
            if q1['cmd'] is not None and q1['container'] is not None:  # 第一个申请是有指令的,直接执行
                return q1
            else:
                idx = find_frist_item(self.gpu_wait_queues[gpuid], need_cmd=True)  # 找到第一个有指令的申请
                if idx == -1 or q1['postponement'] >= 2:  # 都没有指令，或者第一个申请的延迟次数大于等于2次，插入无意义的申请到早上8点
                    end_time = datetime.datetime.now()
                    if end_time.hour >= 22:
                        end_time = end_time + datetime.timedelta(days=1)
                    end_time = end_time.replace(hour=8, minute=0, second=0, microsecond=0)  # 早上8点
                    item = GPURequest(id=-1, duration=0, user='system', container=None, cmd=None, start_time=datetime.datetime.now(), end_time=end_time, postponement=0)
                    self.gpu_wait_queues[gpuid].insert(0, item)
                    self.update_queue()
                    return item
                else:
                    item: GPURequest = self.gpu_wait_queues[gpuid].pop(idx)  # 取出第一个有指令的申请
                    for i in range(idx):
                        self.gpu_wait_queues[gpuid][i]['postponement'] += 1  # 在该指令之前的申请延迟次数加1
                    self.gpu_wait_queues[gpuid].insert(0, item)  # 插入到第一个位置
                    self.update_queue()  # 更新队列时间
                    return item
        else:  # 早上8点到晚上10点优先执行无指令的申请
            q1: GPURequest = self.gpu_wait_queues[gpuid][0]
            if q1['cmd'] is None or q1['container'] is None:  # 第一个申请是无指令的,直接执行
                return q1
            else:
                idx = find_frist_item(self.gpu_wait_queues[gpuid], need_cmd=False)  # 找到第一个无指令的申请
                if idx == -1 or q1['postponement'] >= 2:  # 都有指令，或者第一个申请的延迟次数大于等于2次，直接执行第一个
                    return q1
                else:
                    item: GPURequest = self.gpu_wait_queues[gpuid].pop(idx)  # 取出第一个无指令的申请
                    for i in range(idx):
                        self.gpu_wait_queues[gpuid][i]['postponement'] += 1
                    self.gpu_wait_queues[gpuid].insert(0, item)  # 插入到第一个位置
                    self.update_queue()
                    return item
