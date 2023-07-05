import docker
from docker.client import DockerClient
from docker.models.containers import Container, Image
from io import BytesIO


class DockerController:

    def __init__(self):
        self.client: DockerClient = docker.from_env()

    @property
    def containers(self) -> list[Container]:
        return self.client.containers.list(all=True)

    @property
    def container_running(self) -> list[Container]:
        return self.client.containers.list()

    def get_container(self, container_name: str) -> Container | None:
        try:
            return self.client.containers.get(container_name)
        except docker.errors.NotFound:
            return None

    @property
    def images(self) -> list[Image]:
        return self.client.images.list()

    def get_image(self, image_name: str) -> Image | None:
        try:
            return self.client.images.get(image_name)
        except docker.errors.ImageNotFound:
            return None

    def create_container(self, **kargs) -> Container | docker.errors.ImageNotFound | docker.errors.APIError:
        '''
        kwargs:
            image: str = 'ubuntu:latest',
            name: str = 'container_name',
            command: str|list[str] = '/bin/bash' | ['echo', 'hello world'],
            auto_remove: bool = False,
            dns: list[str] = ['119.29.29.29'.'233.5.5.5'],
            device_requests: list[dict] = [docker.types.DeviceRequest(count=-1, capabilities=[['gpu']])],
            environment: dict|list = {'key': 'value'} | ['key=value'],
            hostname: str = 'container_name',
            shm_size: str = '1G',
            ports: dict = {'80/tcp': 80}|{'80/tcp': ('127.0.0.1', 80)}|{'80/tcp': [1234,5678]}|{'80/tcp': None},
            user: str|int = 'root'|1000 (recommanded),
            volumes: dict = {'/host/path': {'bind': '/container/path', 'mode': 'rw'|'ro'}}|{'/host/path':/container/path},
            working_dir: str = '/path/to/workdir',
        '''
        try:
            return self.client.containers.create(**kargs)
        except docker.errors.ImageNotFound as e:
            return e
        except docker.errors.APIError as e:
            return e

    def create_image(self, dockerfile_path: str, context_path: str, tag: str) -> Image:
        '''
        dockerfile_path: str = 'path/to/dockerfile',
        context_path: str = 'path/to/context',
        tag: str = 'image_name:tag',
        '''
        dockerfile = open(dockerfile_path, 'rb').read()
        return self.client.images.build(fileobj=BytesIO(dockerfile), dockerfile=context_path, tag=tag)[0]

    def push_image(self, image_name: str, tag: str) -> None:
        '''
        image_name: str = 'image_name',
        tag: str = 'tag',
        '''
        resps = self.client.images.push(image_name, tag=tag)

    def pull_image(self, image_name: str, tag: str) -> Image | docker.errors.APIError:
        '''
        image_name: str = 'image_name',
        tag: str = 'tag',
        '''
        try:
            return self.client.images.pull(image_name, tag=tag)
        except docker.errors.APIError as e:
            return e

    def remove_image(self, image_name: str, tag: str) -> bool | docker.errors.APIError:
        '''
        image_name: str = 'image_name',
        '''
        try:
            self.client.images.remove(image_name + ':' + tag)
            return True
        except docker.errors.APIError as e:
            return e
