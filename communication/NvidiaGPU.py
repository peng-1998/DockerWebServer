from typing import List, NamedTuple, Tuple

import pynvml
from pynvml.smi import nvidia_smi

GPUProcess = NamedTuple('GPUProcess', PID=int, memory=float, gpuid=int)


class NVIDIAGPU:

    def __init__(self) -> None:
        self.gpuinstance = nvidia_smi.getInstance()
        self.gpucount = pynvml.nvmlDeviceGetCount()

    def check_gpuid(func) -> None:

        def wrapper(self, gpuid: int):
            if gpuid >= self.gpucount:
                raise ValueError(f'gpuid {gpuid} is not available')
            return func(self, gpuid)

        return wrapper

    @check_gpuid
    def gpu_name(self, gpuid: int = 0) -> str:
        return self.gpuinstance.DeviceQuery()['gpu'][gpuid]['product_name']

    @check_gpuid
    def memory(self, gpuid: int = 0) -> dict:
        """ get the memory of gpu $gpuid

        Args:
            gpuid (int, optional): the id of gpu. Defaults to 0.

        Returns:
            Tuple[float, float]: the total memory and used memory
        """
        total_memory = self.gpuinstance.DeviceQuery('memory.total')['gpu'][gpuid]['fb_memory_usage']['total']
        used_memory = self.gpuinstance.DeviceQuery('memory.used')['gpu'][gpuid]['fb_memory_usage']['used']
        return {'memory_total': total_memory, 'memory_used': used_memory}

    @check_gpuid
    def running_processes(self, gpuid: int = 0) -> List[GPUProcess]:
        """ get the running processes of gpu $gpuid

        Args:
            gpuid (int, optional): the id of gpu. Defaults to 0.

        Returns:
            List[GPUProcess]: the list of GPUProcess
        """
        processes = self.gpuinstance.DeviceQuery('compute-apps')['gpu'][gpuid]['processes']
        if processes is None:
            return []
        return [GPUProcess(int(_['pid']), float(_['used_memory']), gpuid) for _ in processes]

    @check_gpuid
    def utilization(self, gpuid: int = 0) -> int:
        """ get the utilization of gpu $gpuid

        Args:
            gpuid (int, optional): the id of gpu. Defaults to 0.

        Returns:
            float: the utilization of gpu $gpuid 0-100
        """
        return self.gpuinstance.DeviceQuery('utilization.gpu')['gpu'][gpuid]['utilization']['gpu_util']

    @property
    def gpu_info(self) -> dict:
        """ get the info of all gpus

        Returns:
            dict: the info of all gpus like {'0': {'type': 'GeForce RTX 3090', 'memory_total': 24268, 'memory_used': 0, 'utilization': 0}, '1': {'type': 'GeForce RTX 3090', 'memory_total': 24268, 'memory_used': 0, 'utilization': 0}}
        """
        return {i: {'type': self.gpu_name(i), **self.memory(i), 'utilization': self.utilization(i)} for i in range(self.gpucount)}
