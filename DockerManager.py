import os
from typing import List, NamedTuple

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

        return [DockerContainer(container.name, str(container.image)[9:-2], get_port_binding(container.attrs), container.attrs['HostConfig']['Binds'], container.attrs['NetworkSettings']['Networks']['bridge']['IPAddress'], 'running' if container.attrs['State']['Running'] else 'stopped') for container in self.dockerclient.containers.list(all)]

    def image_instantiation(self, container_name: str, user: str, image: str, shm_size: str, volume: List[str], working_dir: str, ports: dict, environment: dict, cmd: str = None) -> None:
        """镜像实例化

        Args:
            container_name (str): 容器名
            user (str): 用户ID
            image (str): 镜像名
            shm_size (str): 共享内存大小 例:'2g'
            volume (List[str]): 挂载目录 例:['/home:/home']
            working_dir (str): 工作目录 例:'/home'
            ports (dict): 端口映射 例:{'8888/tcp': 8888},{'22/tcp':[1234,12345]}
            environment (dict): 环境变量 例:{'PATH': '/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'}
            cmd (str, optional): 启动指令. Defaults to None.
        """
        try:
            self.dockerclient.containers.create(image=image, user=user, command=cmd, working_dir=working_dir, device_requests=[docker.types.DeviceRequest(count=-1, capabilities=[['gpu']])], volumes=volume, shm_size=shm_size, ports=ports, name=container_name, environment=environment)
        except Exception as e:
            print(e)

    def stop_container(self, containername: str) -> None:
        self._find_container(containername).stop()

    def start_container(self, containername: str) -> None:
        self._find_container(containername).start()

    def remove_container(self, containername: str) -> None:
        self._find_container(containername).remove(force=True)

    def _find_container(self, containername: str) -> docker.models.containers.Container:
        return self.dockerclient.containers.get(containername)

    def run_exec(self, containername: str, cmd: str, user: str) -> None:
        try:
            uid = int(os.popen(f'id -u {user}').read().replace('/n', ''))
            self._find_container(containername).exec_run(cmd, False, False, user=str(uid))
        except Exception as e:
            print(e)

    def query_process(self, containername: str) -> List[DockerProcess]:
        """查询容器内进程

        Args:
            containername (str): 容器名

        Returns:
            List[DockerProcess]: 进程列表
        """
        processes = self._find_container(containername).top(ps_args='-eo pid,ppid,size,cmd')['Processes']
        return [DockerProcess(*_) for _ in processes]
