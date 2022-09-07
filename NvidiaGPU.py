from typing import List, NamedTuple, Tuple

import pynvml
from pynvml.smi import nvidia_smi

GPUProcess = NamedTuple('GPUProcess', PID=int, memory=float)


class NVIDIA_GPU:

    def __init__(self) -> None:
        self.gpuinstance = nvidia_smi.getInstance()
        self.gpucount = pynvml.nvmlDeviceGetCount()

    def gpu_name(self, gpuid: int = 0) -> str:
        self.check_gpuid(gpuid)
        return self.gpuinstance.DeviceQuery()['gpu'][gpuid]['product_name']

    def memory(self, gpuid: int = 0) -> Tuple[str, str]:
        self.check_gpuid(gpuid)
        return str(int(self.gpuinstance.DeviceQuery('memory.used')['gpu'][gpuid]['fb_memory_usage']['used'])), str(int(self.gpuinstance.DeviceQuery('memory.total')['gpu'][gpuid]['fb_memory_usage']['total']))

    def check_gpuid(self, gpuid: int) -> None:
        assert isinstance(gpuid, int) and gpuid >= 0 and gpuid < self.gpucount

    def running_processes(self, gpuid: int = 0) -> List[GPUProcess]:
        self.check_gpuid(gpuid)
        processes = self.gpuinstance.DeviceQuery('compute-apps')['gpu'][gpuid]['processes']
        if processes is None:
            return []
        return [GPUProcess(int(_['pid']), float(_['used_memory'])) for _ in processes]

    def gpu_info(self) -> List[Tuple]:
        return [{'gpuid': str(i), 'gputype': self.gpu_name(i), 'memory': '/'.join([*self.memory(i)]) + ' MB'} for i in range(self.gpucount)]
