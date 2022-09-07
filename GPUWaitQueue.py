import datetime
from typing import List, TypedDict

import config

GPURequest = TypedDict('GPURequest', duration=int, user=str, cmd=str, container=str, start_time=datetime.datetime, reason=str, end_time=datetime.datetime, dalay_time=int)


class GPUWaitQueue:

    def __init__(self, gpucount: int = 1) -> None:
        self.gpucount = gpucount
        self.gpu_wait_queues: List[List[GPURequest]] = [[] for _ in range(gpucount)]
        if config.scheduling_policy == 'FIFO':
            self.scheduler = lambda gpuid: self.gpu_wait_queues[gpuid][0] if len(self.gpu_wait_queues[gpuid]) != 0 else None
        elif config.scheduling_policy == 'WNCFNWCF':
            self.scheduler = lambda gpuid: self.wncfnwcf(gpuid)

    def new_item(self, gpuid: int, user: str, duration: int, reason: str, container: str = None, cmd: str = None) -> GPURequest:
        current_queue = self.gpu_wait_queues[gpuid]
        if len(current_queue) == 0:
            start_time = datetime.datetime.now()  # 如果队列为空，直接开始
        else:
            start_time = current_queue[-1]['end_time']  # 如果队列不为空，等待队列最后一个结束
        end_time = start_time + datetime.timedelta(hours=duration)  # 结束时间
        item = GPURequest(duration=duration, user=user, reason=reason, container=container, cmd=cmd, start_time=start_time, end_time=end_time, dalay_time=0)
        current_queue.append(item)
        return item

    def current_item(self) -> List(GPURequest):
        return [queue[0] if len(queue) != 0 else None for queue in self.gpu_wait_queues]

    def update_queue(self) -> None:
        for queue in [queue for queue in self.gpu_wait_queues if len(queue) != 0]:
            pre = head = queue[0]
            if head['start_time'] > datetime.datetime.now():
                head['start_time'] = datetime.datetime.now()
                head['end_time'] = head['start_time'] + datetime.timedelta(hours=head['duration'])
            for item in queue[1:]:
                item['start_time'] = pre['end_time']
                item['end_time'] = item['start_time'] + datetime.timedelta(hours=item['duration'])
                pre = item

    def stop(self, gpuid: int) -> GPURequest:
        """某一申请结束时的逻辑,执行时机:1 申请时间到达 2 从容器当中发出的通知 3 web端发出的通知

        Args:
            gpuid (int): GPU id
        """

        self.gpu_wait_queues[gpuid].pop(0)  # 当前执行的申请出队列
        if len(self.gpu_wait_queues[gpuid]) == 0:  # 没有需要执行的申请
            return None

        self.update_queue()  # 更新队列时间
        return self.scheduler(gpuid)  # 返回下一个需要执行的申请

    def wncfnwcf(self, gpuid: int) -> GPURequest:

        def find_frist_item(queue: List, need_cmd: bool = True) -> int:
            for idx, item in enumerate(queue):
                if (need_cmd and item['cmd'] is not None and item['container'] is not None) or (not need_cmd and item['cmd'] is None):
                    return idx
            return -1

        now_time_hour = datetime.datetime.now().hour
        if now_time_hour < config.worktime_start or now_time_hour > config.worktime_end:  # 非工作时间不执行无指令的申请
            head: GPURequest = self.gpu_wait_queues[gpuid][0]
            if head['cmd'] is not None and head['container'] is not None:  # 第一个申请是有指令的,直接执行
                return head
            else:
                idx = find_frist_item(self.gpu_wait_queues[gpuid], need_cmd=True)  # 找到第一个有指令的申请
                if idx == -1 or head['dalay_time'] >= config.max_dalay_time:  # 都没有指令，或者第一个申请的延迟次数大于等于n次，插入无意义的申请到早上8点
                    end_time = datetime.datetime.now()
                    if end_time.hour >= config.worktime_end:
                        end_time = end_time + datetime.timedelta(days=1)
                    end_time = end_time.replace(hour=config.worktime_start, minute=0, second=0, microsecond=0)
                    item = GPURequest(duration=0, user='system', reason='空等待', container=None, cmd=None, start_time=datetime.datetime.now(), end_time=end_time, dalay_time=0)
                else:
                    item: GPURequest = self.gpu_wait_queues[gpuid].pop(idx)  # 取出第一个有指令的申请
                    for i in range(idx):
                        self.gpu_wait_queues[gpuid][i]['dalay_time'] += 1  # 在该指令之前的申请延迟次数加1
                self.gpu_wait_queues[gpuid].insert(0, item)  # 插入到第一个位置
                self.update_queue()  # 更新队列时间
                return item
        else:  # 工作时间优先执行无指令的申请
            head: GPURequest = self.gpu_wait_queues[gpuid][0]
            if head['cmd'] is None or head['container'] is None:  # 第一个申请是无指令的,直接执行
                return head
            else:
                idx = find_frist_item(self.gpu_wait_queues[gpuid], need_cmd=False)  # 找到第一个无指令的申请
                if idx == -1 or head['dalay_time'] >= config.max_dalay_time:  # 都有指令，或者第一个申请的延迟次数大于等于n次，直接执行第一个
                    return head
                else:
                    item: GPURequest = self.gpu_wait_queues[gpuid].pop(idx)  # 取出第一个无指令的申请
                    for i in range(idx):
                        self.gpu_wait_queues[gpuid][i]['dalay_time'] += 1
                    self.gpu_wait_queues[gpuid].insert(0, item)  # 插入到第一个位置
                    self.update_queue()
                    return item
