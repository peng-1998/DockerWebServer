import os
from typing import NamedTuple,List
import docker

DockerProcess = NamedTuple('DockerProcess', PID=int, PPID=int, size=float, cmd=str)
DockerContainer = NamedTuple('DockerContainer', name=str, image=str, ports=dict, volume=list, ip=str, status=str)


class DockerManager:

    def __init__(self) -> None:
        self.dockerclient = docker.from_env()

    @property
    def images(self) -> List[str]:
        return [str(image)[9:-2] for image in self.dockerclient.images.list()]

    @property
    def running_containers(self) -> List[DockerContainer]:
        return self.get_containers()

    @property
    def all_containers(self) -> List[DockerContainer]:
        return self.get_containers(True)

    def get_containers(self, all: bool = False) -> List[DockerContainer]:

        def get_port_binding(attrs):
            port_bindings = attrs['HostConfig']['PortBindings']
            return [{'container_port': int(key.split('/')[0]), 'host_port': int(port_bindings[key][0]['HostPort'])} for key in port_bindings]

        return [
            DockerContainer(container.name,
                            str(container.image)[9:-2], get_port_binding(container.attrs), container.attrs['HostConfig']['Binds'], container.attrs['NetworkSettings']['Networks']['bridge']['IPAddress'], 'running' if container.attrs['State']['Running'] else 'stopped')
            for container in self.dockerclient.containers.list(all)
        ]

    def image_instantiation(self, container_name: str, user: str, image: str, shm_size: str, volume: List[str], working_dir: str, ports: dict, environment: dict , cmd: str  = None) -> None:
        '''
        Args:
            container_name:容器名 e.g. 'my_container'
            image:镜像 e.g. 'ubuntu:lastest'
            shm_size:共享内存大小 e.g. '2g'
            volume:目录映射列表(主机:容器) e.g. ['/home/A:/root/A','/home/B:/root/C'] 
            working_dir:默认工作目录 e.g. '/root/working_dir'
            ports:端口映射 (容器:主机) e.g. {'22/tcp':1234,'5555/tcp':6660} or {'22/tcp':[1234,12345]}
            environment: 环境变量 e.g. ['SERVER_USER=tom']
            cmd:启动时命令 e.g. 'bash'
        '''
        try:
            self.dockerclient.containers.create(image=image, user=user, command=cmd, working_dir=working_dir, device_requests=[docker.types.DeviceRequest(count=-1, capabilities=[['gpu']])], volumes=volume, shm_size=shm_size, ports=ports, name=container_name, environment=environment)
        except:
            ...

    def stop_container(self, containername: str) -> None:
        self.find_container(containername).stop()

    def start_container(self, containername: str) -> None:
        self.find_container(containername).start()

    def remove_container(self, containername: str) -> None:
        self.find_container(containername).remove(force=True)

    def find_container(self, containername: str):
        return self.dockerclient.containers.get(containername)

    def run_exec(self, containername: str, cmd: str,user:str):
        try:
            uid = int(os.popen(f'id -u {user}').read().replace('/n',''))
            self.find_container(containername).exec_run(cmd, False,False,user=str(uid))
        except:
            ...

    def query_process(self, containername: str) -> List[DockerProcess]:
        '''
        return List of [pid,size(kb),cmd]
        '''
        processes = self.find_container(containername).top(ps_args='-eo pid,ppid,size,cmd')['Processes']
        return [DockerProcess(*_) for _ in processes]