# This is the more standardised database module, which is based on Flask-SQLAlchemy.
# db_path: sqlite:///mydatabase.db 
from flask_sqlalchemy import SQLAlchemy

db = SQLAlchemy()

# 表对象：用户、镜像、容器、设备
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    account = db.Column(db.String(20), unique=True, nullable=False)
    nickname = db.Column(db.String(20))
    password = db.Column(db.String(20), nullable=False)
    email = db.Column(db.String(20))
    phone = db.Column(db.String(20))
    photo = db.Column(db.String(30))

    def __init__(self, account, nickname, password, email, phone, photo):
        self.account = account
        self.nickname = nickname
        self.password = password
        self.email = email
        self.phone = phone
        self.photo = photo
    
class Image(db.Model):
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    showname = db.Column(db.String(20), nullable=False)
    imagename = db.Column(db.String(20), nullable=False)
    init_args = db.Column(db.PickleType)
    description = db.Column(db.String(20))

    def __init__(self, showname, imagename, init_args, description):
        self.showname = showname
        self.imagename = imagename
        self.init_args = init_args
        self.description = description

class Container(db.Model):
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    showname = db.Column(db.String(20), nullable=False)
    containername = db.Column(db.String(20), nullable=False)
    machineid = db.Column(db.Integer, nullable=False)
    portlist = db.Column(db.PickleType)
    running = db.Column(db.Boolean)
    imageid = db.Column(db.Integer, db.ForeignKey('image.id'))
    machineid = db.Column(db.Integer, db.ForeignKey('machine.id'))
    userid = db.Column(db.Integer, db.ForeignKey('user.id'))

    def __init__(self, showname, containername, machineid, portlist, running, imageid, userid):
        self.showname = showname
        self.containername = containername
        self.machineid = machineid
        self.portlist = portlist
        self.running = running
        self.imageid = imageid
        self.machineid = machineid
        self.userid = userid

class Machine(db.Model):
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    ip = db.Column(db.String(20))
    gpu = db.Column(db.PickleType)
    disk = db.Column(db.PickleType)
    cpu = db.Column(db.PickleType)
    memory = db.Column(db.PickleType)
    online = db.Column(db.Boolean)

    def __init__(self, ip, gpu, disk, cpu, memory, online):
        self.ip = ip
        self.gpu = gpu
        self.disk = disk
        self.cpu = cpu
        self.memory = memory
        self.online = online