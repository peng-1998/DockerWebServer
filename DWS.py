import datetime
import json
import os
import time

from flask import Flask, flash, redirect, request
from tinydb import Query, TinyDB

import config
from DockerManager import DockerManager
from GPUQueueManager import GPUQueueManager, GPURequest
from MailSender import EmailMessager
from NvidiaGPU import NVIDIA_GPU
from TimeManager import TimeManager
from utils import TestContainer, TestPasswd

database = TinyDB('data/database/database.json')
# 初始化
app = Flask("DSM")
app.secret_key = 'DSM'
# 加载数据库
user_info_database = database.table('user_info')
docker_image_database = database.table('docker_images')
gpu_request_database = database.table('gpu_request_database')
query = Query()
# 创建管理器
nvidia_gpu = NVIDIA_GPU()
gpu_queue_manager = GPUQueueManager(nvidia_gpu.gpucount)
docker_manager = DockerManager()
## 读取所有用户邮件信息
user_email = {}
for info in user_info_database.all():
    if info['email'] != None:
        user_email[info['user']] = info['email']
user_email['administrator'] = config.mail_sender
neteasy_email_manager = EmailMessager(config.mail_login_user, config.mail_password, config.mail_sender, user_email, config.from_str, config.server_host, config.server_port, config.use_ssl, config.max_resend_time)
## 读取所有镜像名称
image_name_dict = {}
for image in docker_image_database.all():
    image_name_dict[image['name']] = image['show_name']
## 读取用户昵称
user2name = {}
for info in user_info_database.all():
    if info['name'] is not None:
        user2name[info['user']] = info['name']
time_manager = TimeManager(nvidia_gpu, docker_manager, gpu_queue_manager, neteasy_email_manager)


@app.before_first_request
def init():
    time_manager.start()


@app.route('/')
def index():
    if 'uname' in request.cookies:  # cookie中有uname项, 说明已经登录
        return redirect('/static/user_page.html')  # 跳转到用户页面
    else:
        return redirect('/login')  # 跳转到登录页面


@app.route('/login', methods=('GET', 'POST'))
def login():
    if request.method == 'POST':
        if request.form['key'] == 'login':
            user_name = request.form.get('account')
            passward = request.form.get('password')
            hold_login = request.form.get('holdlogin')
            if TestPasswd(user_name, passward):
                resp = redirect('/')
                if hold_login == 'True':
                    resp.set_cookie('uname', user_name, 60 * 60 * 24 * 30)
                else:
                    resp.set_cookie('uname', user_name, 60 * 60)
                return resp
            else:
                flash("该用户名不存在或密码错误")
                return redirect('/login')
    return redirect('/static/login.html')


@app.route('/gpumanager/gpuinfo')
def gpuinfo():
    gpu_info = nvidia_gpu.gpu_info()  # 获取GPU信息
    for i in range(nvidia_gpu.gpucount):
        ginfo = gpu_info[i]
        waitlist = gpu_queue_manager.gpu_wait_queues[i]
        if len(waitlist) == 0:
            ginfo['currentuser'] = '无'
            ginfo['waittime'] = '无'
        else:
            ginfo['currentuser'] = user2name[waitlist[0]['user']] if waitlist[0]['user'] in user2name else waitlist[0]['user']
            d: datetime.timedelta = waitlist[-1]['end_time'] - datetime.datetime.now()  # 计算等待时间
            ginfo['waittime'] = f'{int(d.total_seconds()//3600)}小时{int((d.total_seconds()%3600)//60)}分钟'
    return json.dumps(gpu_info)


@app.route('/gpumanager/waitlist')
def waitlist():
    reslist = []
    for idx, queue in enumerate(gpu_queue_manager.gpu_wait_queues):
        for item in queue:
            dbid = item['id']
            dbitem = gpu_request_database.search(query.id == dbid)[0]
            reslist.append({'user': user2name[item['user']] if item['user'] in user2name else item['user'], 'gpuid': idx, 'duration': item['duration'], 'reason': dbitem.reason, 'container': '无' if item['container'] is None else item['container'], 'cmd': '无' if item['cmd'] is None else item['cmd']})
    return json.dumps(reslist)


@app.route('/gpumanager/currentitem')
def currentitem():
    r = []  # retuen list
    uname = request.cookies.get('uname')
    for idx, queue in enumerate(gpu_queue_manager.gpu_wait_queues):
        for item in queue:
            if item['user'] == uname:
                r_dict = {'gpuid': idx, 'duration': item['duration']}
                if item['start_time'] < datetime.datetime.now():
                    r_dict['status'] = '正在使用'
                    d: datetime.timedelta = item['end_time'] - datetime.datetime.now()
                    r_dict['timeinfo'] = f'{int(d.total_seconds()//3600)}小时{int((d.total_seconds()%3600)//60)}分钟后结束'
                else:
                    r_dict['status'] = '排队等待'
                    d = item['start_time'] - datetime.datetime.now()
                    r_dict['timeinfo'] = f'{int(d.total_seconds()//3600)}小时{int((d.total_seconds()%3600)//60)}分钟后可用'
                r.append(r_dict)
    return json.dumps(r)


@app.route('/gpumanager/applyforgpu', methods=['POST'])
def applyforgpu():
    attrs = request.json
    uname = request.cookies.get('uname')
    id = len(gpu_request_database.all())
    container, cmd = attrs['container'], attrs['cmd']
    if container == "选择容器" or cmd == '':  # 没有设置容器和命令
        container = None
        cmd = None
    item: GPURequest = gpu_queue_manager.new_item(id, int(attrs['gpuid']), uname, int(attrs['duration']), container, cmd)
    gpu_request_database.insert({'id': id, 'user': uname, 'gpuid': int(attrs['gpuid']), 'duration': int(attrs['duration']), 'reason': attrs['reason'], 'container': container, 'cmd': cmd})
    r_str = '申请成功!'
    start_time: datetime.datetime = item['start_time']
    if start_time < datetime.datetime.now():
        r_str += '你获得GPU使用权限.'
        if container is not None and cmd is not None:
            docker_manager.run_exec(item['container'], item['cmd'], item['user'])
            r_str += '你托管的指令已经执行.'
        else:
            r_str += '请抓紧时间利用GPU.'
    else:
        r_str += f'正在排队等待GPU,GPU预计将于{start_time.year}年{start_time.month}月{start_time.day}日{start_time.hour}时{start_time.minute}分可用.'
        if container is not None and cmd is not None:
            r_str += '你托管的指令将在GPU可用时自动执行.'
        else:
            r_str += '你没有托管指令,系统将在GPU可用时通过电子邮件通知.'
    return r_str


@app.route('/gpumanager/stopearlycontainer')
def stopearlycontainer():
    user = request.args.get('user')
    ip_addr = request.remote_addr
    containers = docker_manager.get_containers()
    for container in containers:
        if TestContainer(container.name, user):
            if container.ip == ip_addr:
                for idx, item in enumerate(gpu_queue_manager.current_item()):
                    if item is not None and item['user'] == user:
                        if item['user'] in neteasy_email_manager.user_email:
                            neteasy_email_manager.send(f"{datetime.datetime.now().strftime( '%Y-%m-%d %H:%M:%S %f' )}程序运行完成通知", "你运行的实验进程已经运行完成，GPU不再可用，再次使用请重新申请。", item['user'])
                        item: GPURequest = gpu_queue_manager.stop(idx)
                        if item is not None:
                            if item['container'] is not None and item['cmd'] is not None:
                                docker_manager.run_exec(item['container'], item['cmd'], item['user'])
                                if item['user'] in neteasy_email_manager.user_email:
                                    neteasy_email_manager.send(f"{datetime.datetime.now().strftime( '%Y-%m-%d %H:%M:%S %f' )}GPU可用通知", f"你在服务器上预约的GPU已经可以使用,你托管的启动命令已经启动。你申请的使用时长为{item['duration']}小时,可用时间为{item['start_time'].strftime( '%Y-%m-%d %H:%M:%S %f' )}到{item['end_time'].strftime( '%Y-%m-%d %H:%M:%S %f' )}.", item['user'])
                            else:
                                if item['user'] in neteasy_email_manager.user_email:
                                    neteasy_email_manager.send(f"{datetime.datetime.now().strftime( '%Y-%m-%d %H:%M:%S %f' )}GPU可用通知", f"你在服务器上预约的GPU已经可以使用,请尽快登录容器完成实验。你申请的使用时长为{item['duration']}小时,可用时间为{item['start_time'].strftime( '%Y-%m-%d %H:%M:%S %f' )}到{item['end_time'].strftime( '%Y-%m-%d %H:%M:%S %f' )}.", item['user'])
                        return "当前任务已结束"
    return "IP 验证错误"


@app.route('/gpumanager/stopearlyweb', methods=['POST'])
def stopearlyweb():
    attrs = request.json
    gpuid = int(attrs['gpuid'])
    uname = request.cookies.get('uname')
    if len(gpu_queue_manager.gpu_wait_queues[gpuid]) == 0 or gpu_queue_manager.gpu_wait_queues[gpuid][0]['user'] != uname:
        return "消息过时，请刷新网页！"
    item: GPURequest = gpu_queue_manager.stop(gpuid)
    if item is not None:
        if item['container'] is not None and item['cmd'] is not None:
            docker_manager.run_exec(item['container'], item['cmd'], item['user'])
            if item['user'] in neteasy_email_manager.user_email:
                neteasy_email_manager.send(f"{datetime.datetime.now().strftime( '%Y-%m-%d %H:%M:%S %f' )}GPU可用通知", f"你在服务器上预约的GPU已经可以使用,你托管的启动命令已经启动。你申请的使用时长为{item['duration']}小时,可用时间为{item['start_time'].strftime( '%Y-%m-%d %H:%M:%S %f' )}到{item['end_time'].strftime( '%Y-%m-%d %H:%M:%S %f' )}.", item['user'])
        else:
            if item['user'] in neteasy_email_manager.user_email:
                neteasy_email_manager.send(f"{datetime.datetime.now().strftime( '%Y-%m-%d %H:%M:%S %f' )}GPU可用通知", f"你在服务器上预约的GPU已经可以使用,请尽快登录容器完成实验。你申请的使用时长为{item['duration']}小时,可用时间为{item['start_time'].strftime( '%Y-%m-%d %H:%M:%S %f' )}到{item['end_time'].strftime( '%Y-%m-%d %H:%M:%S %f' )}.", item['user'])
    return "当前任务已结束"


@app.route('/gpumanager/cancleapplyfor', methods=['POST'])
def cancleapplyfor():
    attrs = request.json
    uname = request.cookies.get('uname')
    gpuid = int(attrs['gpuid'])
    queue = gpu_queue_manager.gpu_wait_queues[gpuid]
    for i in range(len(queue)):
        if queue[i]['user'] == uname:
            queue.pop(i)
            break
    return "申请已经撤销。"


def remove_reapeted_request(requests: list):
    r = []
    for item in requests:
        if item not in r:
            r.append(item)
    return r


@app.route('/gpumanager/requesthistory')
def requesthistory():
    uname = request.cookies.get('uname')
    r = []
    for item in gpu_request_database.search(query.user == uname):
        r.append({"gpuid": item['gpuid'], "duration": item['duration'], "reason": item['reason'], "cmd": item['cmd']})
    r = remove_reapeted_request(r)
    return json.dumps(r[::-1])


@app.route('/containermanager/imagelist')
def imagelist():
    r = []
    for image in docker_image_database.all():
        r.append({"imagename": image['show_name'], "env": image['env_description'], "useguide": image['use_description']})
    return json.dumps(r)


@app.route('/containermanager/mycontainer')
def mycontainer():
    uname = request.cookies.get('uname')
    containers = docker_manager.all_containers
    r = []
    for container in containers:
        if TestContainer(container.name, uname):
            r.append({"containername": container.name, "imagename": image_name_dict[container.image], "status": container.status, "ports": '|'.join([f"{port['container_port']}->{port['host_port']}" for port in container.ports])})

    return json.dumps(r)


@app.route('/containermanager/applyforcontainer', methods=['POST'])
def applyforcontainer():
    attrs = request.json
    uname = request.cookies.get('uname')
    imagename = attrs['imagename']
    ports = attrs['ports']
    containers = docker_manager.all_containers
    ports_used = []
    # 查找该用户已有的容器
    r = []
    for container in containers:
        if TestContainer(container.name, uname):
            r.append(container.name)
        # 统计在docker系统当中已经被占用的端口
        ports_used += [_['host_port'] for _ in container.ports]
    # 选择不存在的容器名
    for i in ['a', 'b', 'c']:
        if f'{uname}_{i}' not in r:
            containername = f'{uname}_{i}'
            break
    # 处理额外端口
    if ports is not None and ports != '':
        ports = [int(_) for _ in ports.split('|')]
    else:
        ports = []
    # 查找对应的镜像信息
    image = docker_image_database.search(query.show_name == imagename)[0]
    # 获取镜像的默认端口信息
    default_ports = image['default_ports']
    image: str = image['name']
    # 处理创建容器需要的文件
    if os.path.exists(f'data/user/{uname}/{containername}'):  # 如果存在旧文件则删除
        os.system(f'rm -r data/user/{uname}/{containername}')
    os.makedirs(f'data/user/{uname}/{containername}')
    # 拷贝四个文件
    os.system(f'cp data/image/{image}/group data/user/{uname}/{containername}')
    os.system(f'cp data/image/{image}/shadow data/user/{uname}/{containername}')
    os.system(f'cp data/image/{image}/passwd data/user/{uname}/{containername}')
    os.system(f'cp data/image/{image}/sudoers data/user/{uname}/{containername}')
    time.sleep(0.05)
    # 写入用户信息
    os.system(f'echo {uname}:x:$(id -g {uname}) >> data/user/{uname}/{containername}/group')  # 添加用户组
    os.system(f'echo $(cat /etc/shadow | grep {uname}:) >> data/user/{uname}/{containername}/shadow')  # 设置密码
    os.system(f'echo $(cat /etc/passwd | grep {uname}:) >> data/user/{uname}/{containername}/passwd')  # 设置用户
    os.system(f'echo "{uname} ALL=(ALL:ALL) NOPASSWD: ALL" >> data/user/{uname}/{containername}/sudoers')  # 设置sudo权限
    # 将以上文件的创建者设置为root
    os.system(f'chown -R root:root data/user/{uname}/{containername}/*')
    # 拷贝启动脚本并将所有者设置为当前用户
    os.system(f'cp data/image/{image}/start.sh data/user/{uname}/{containername}')
    os.system(f'chown {uname}:{uname} data/user/{uname}/{containername}/start.sh')
    # 如果没有workspace文件夹则创建
    if not os.path.exists(f'/data/{uname}/workspace'):
        os.makedirs(f'/data/{uname}/workspace')
        os.system(f'chown -R {uname} /data/{uname}/workspace')
    # 所有路径转化为绝对路径
    group_path = os.path.abspath(f'data/user/{uname}/{containername}/group')
    shadow_path = os.path.abspath(f'data/user/{uname}/{containername}/shadow')
    passwd_path = os.path.abspath(f'data/user/{uname}/{containername}/passwd')
    sudoers_path = os.path.abspath(f'data/user/{uname}/{containername}/sudoers')
    workspace = os.path.abspath(f'/data/{uname}/workspace')
    start_file = os.path.abspath(f'data/user/{uname}/{containername}/start.sh')
    # 所有需要映射的端口
    for port in default_ports:
        if port not in ports:
            ports.append(port)
    # 查找未使用的主机端口
    host_ports = []
    for i in range(1000):
        if i + 10000 not in ports_used:
            host_ports.append(i + 10000)
        if len(host_ports) == len(ports):
            break
    # 设置文件映射
    volumes = {}
    volumes[group_path] = {'bind': '/etc/group', 'mode': 'ro'}
    volumes[shadow_path] = {'bind': '/etc/shadow', 'mode': 'ro'}
    volumes[passwd_path] = {'bind': '/etc/passwd', 'mode': 'ro'}
    volumes[sudoers_path] = {'bind': '/etc/sudoers', 'mode': 'ro'}
    volumes[workspace] = {'bind': f'/home/{uname}/workspace', 'mode': 'rw'}
    volumes[start_file] = {'bind': f'/home/{uname}/start.sh', 'mode': 'ro'}
    # 设置端口映射
    ports_ = {}
    for p, q in zip(ports, host_ports):
        ports_[f'{p}/tcp'] = q
    # 容器内用户的uid
    uid = int(os.popen(f'id -u {uname}').read().replace('/n', ''))
    # 创建容器
    docker_manager.image_instantiation(containername, uid, image, '4g', volumes, f'/home/{uname}/workspace', ports_, None, cmd=f'bash /home/{uname}/start.sh')
    return "容器创建完成."


@app.route('/containermanager/containeropt', methods=['POST'])
def containeropt():
    # 执行容器启动/停止/删除操作
    attrs = request.json
    containername = attrs['containername']
    opt = attrs['opt']
    if opt == 'stop':
        docker_manager.stop_container(containername)
    if opt == 'start':
        docker_manager.start_container(containername)
    if opt == 'remove':
        docker_manager.remove_container(containername)
    return "容器操作完成,请等待5s再进行操作."


@app.route('/setting/loademail')
def loademail():
    uname = request.cookies.get('uname')
    user = user_info_database.search(query.user == uname)
    if len(user) == 0:
        return 'None'
    return user[0]['email']


@app.route('/setting/setemail', methods=['POST'])
def setemail():
    uname = request.cookies.get('uname')
    query = user_info_database.search(query.name == uname)
    email = request.json['email']
    if len(query) == 0:
        user_info_database.insert({'user': uname, 'name': None, 'email': email})
    else:
        query[0]['email'] = email
    neteasy_email_manager.user_email[uname] = email
    return "成功"


@app.route('/setting/loadname')
def loadname():
    uname = request.cookies.get('uname')
    user = user_info_database.search(query.user == uname)
    if len(user) == 0 or user[0]['name'] is None:
        return 'None'
    return user[0]['name']


@app.route('/setting/setname', methods=['POST'])
def setname():
    uname = request.cookies.get('uname')
    user = user_info_database.search(query.user == uname)
    name = request.json['name']
    user2name[uname] = name
    if len(user) == 0:
        user_info_database.insert({'user': uname, 'name': name, 'email': None})
    else:
        user[0]['name'] = name
    return "成功"


@app.route('/setting/suggest', methods=['POST'])
def suggest():
    uname = request.cookies.get('uname')
    content = request.json['content']
    neteasy_email_manager.send(uname + "的留言", content, 'administrator')
    return "提交成功!"


@app.route('/setting/addimages', methods=['GET'])
def addimages():
    ...


@app.route('/setting/addimages', methods=['POST'])
def addimages():
    image = request.json['image']
    passwd = request.json['passwd']
    if passwd != config.administrator_passwd:
        return "密码错误"
    docker_image_database.insert(image)
    image_name_dict[image['name']] = image['show_name']
    return "添加成功"


@app.route('/setting/quit')
def quit():
    uname = request.cookies.get('uname')
    resp = redirect('/')
    resp.set_cookie('uname', uname, 0)
    return resp


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=config.serverPort, debug=False, threaded=True)
