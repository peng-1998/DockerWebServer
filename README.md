# DockerWebServer

为实验室环境设计的GPU服务器管理系统，基于Docker容器技术，支持多用户同时使用。

目前只支持单GPU服务器，多GPU服务器的支持正在开发中。

特点：
- 每个使用者可以在服务器上创建自己的容器，容器内的环境由管理员统一管理，使用者只需要在容器内安装自己需要的软件即可。

- GPU采用独占模式进行分配，申请GPU的用户可以按照队列的顺序使用GPU。

- 可以托管训练指令，当分配给该用户GPU的时候，自动执行训练指令。


使用的容器镜像为定制镜像，容器内部和宿主机当中的用户相同，因此读写权限相同。

实现相同权限的关键是在容器内安装Sudo软件包，并在启动脚本时指定用户UID和共享group,passwd,shadow等文件的相关内容。

镜像定制可以参考[这里](https://github.com/peng-1998/aliyun_docker_images/tree/main/ubuntu）。


## 使用

首先在config.py中配置好服务器的相关信息，然后运行DWS.py即可。

```
python DWS.py
```

关于wiki部分，这里使用的是markdown文档static/食用指北.md编译的html网页，地址是static/食用指北.html，可以自行修改。


