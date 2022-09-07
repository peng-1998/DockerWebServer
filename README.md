# DockerWebServer

主要代码逻辑：钟云鹏

UI：周豪哲

# 使用

## 依赖

### python库
此项目使用flask框架作为后端, 并使用TinyDB存储数据.
同时为了获取docker容器以及GPU情况, 还依赖于docker以及pynvml.

### 软件

项目依赖于docker和nvidia-smi获取数据, 因此请确保docker命令与nvidia-smi命令可用.

## 权限

如果你无法以root身份运行此程序:
  + 项目使用linux用户及密码进行登陆, 需要读取/etc/shadow文件以验证密码, 运行时请确保运行用户拥有读取权限
  + 程序需要访问docker访问的权限, 请保证运行用户在docker用户组当中

## 镜像

我们为容器设计了一个特殊配置, 以保证容器与宿主机当中用户拥有相同的权限, 因此对镜像环境有要求.
最主要是要求容器当中安装sudo包.

接着, 需要从该镜像当中取出/etc/passwd, /etc/group, /etc/shadow, /etc/sudoers四个文件并放入data/image/{image name}/当中. 同时编写start.sh脚本, 主要工作为创建/home/{user}目录并使用shown命令将创建者变更为指定user, 并在最后执行一个一直运行的进程来保证容器处于运行状态(如:while true; do sleep 10000000; done). 可以参考data/image/conda:py38_492/start.sh

镜像构建可以参考dockerfile/Dockerfile.servermanager_pytorch

# 更新日志

20220901：
  + 添加无指令请求在非工作时间的顺延逻辑
  + 添加工作时间优先执行无指令申请的逻辑
  + 添加防止饥饿的逻辑
  + 修改了一系列BUG, 这一系列BUG曾导致：某些用户可以看到其他用户的容器并使用, 某些用户申请的可执行时间内某些特定用户可以使用GPU
  + 去除了历史申请队列当中的重复项

20220903:
  + 使用TinyDB代替原有的数据存储
  + 更新了登陆UI

20220906:
  + 修复了一个bug, 该bug曾导致自然结束的申请未被结束
  + 修复了一个容器bug, 该bug曾导致容器启动后自动关闭
  + 发现一个待修复的UI bug, 该bug导致容器管理管理按钮除了最下面的按钮无法使用

20220907:
  + 创建devel分支
  
