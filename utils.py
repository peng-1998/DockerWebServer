import crypt
import hashlib
from config import container_flags

def TestPasswd(username: str, passwd: str) -> bool:
    passFile = open('/etc/shadow') # 需要root权限 或者单独为这个用户给读权限
    for line in passFile.readlines():
        if ":" in line:
            user = line.split(":")[0]
            if user != username:
                continue
            cryptPass = line.split(":")[1].strip(' ')
            salt = cryptPass[cryptPass.find("$"):cryptPass.rfind("$")]
            cryptWord = crypt.crypt(passwd, salt)
            return cryptWord == cryptPass
    return False


# 判断容器是否属于某个用户
def TestContainer(container: str, username: str) -> bool:
    if '_' in container:
        uname, id = container.split('_')
        return uname == username and id in ['a', 'b', 'c']
    return False


def user2md5(uname: str):
    m = hashlib.md5()
    m.update('uname={uname}'.encode('utf-8'))
    return m.hexdigest()


def TestUser(uname: str, hashcode: str) -> bool:
    return user2md5(uname) == hashcode
