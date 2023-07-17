from abc import ABC, abstractmethod
from typing import Any, OrderedDict


class SchedulingStrategy(ABC):

    @abstractmethod
    def __call__(self, wait_queue: OrderedDict, source: dict) -> list[dict]:
        ...


class FCFS(SchedulingStrategy):

    def __call__(self, wait_queue: OrderedDict, source: dict) -> list[dict]:
        if not wait_queue or not any(source.values()):
            return []
        
        res = []
        source_ = source.copy()

        for key, value in wait_queue.items():
            if not any(source.values()):
                break

            gpu = value['GPU']

            if isinstance(gpu, list) and all(source_[g] for g in gpu):
                res.append({'task_id': key, 'GPU': gpu})
                for g in gpu:
                    source_[g] = False

            else:
                available = [k for k, v in source_.items() if v]
                if len(available) >= gpu:
                    res.append({'task_id': key, 'GPU': available[:gpu]})
                    for g in available[:gpu]:
                        source_[g] = False

        return res
