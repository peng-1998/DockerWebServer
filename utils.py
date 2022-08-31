import crypt

def TestPasswd(username: str, passwd: str) -> bool:
    passFile = open('/etc/shadow')
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

