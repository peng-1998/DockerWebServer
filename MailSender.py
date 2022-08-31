import smtplib
from email.mime.text import MIMEText
import time

class NeteasyEmailMessager:

    def __init__(self, mail_user: str, mail_pass: str, user_email: dict):
        self.mail_user = mail_user
        self.mail_pass = mail_pass
        self.user_email = user_email
        self.sender = self.mail_user + '@m.scnu.edu.cn'
        self.send_queue = []

    def send(self, subject: str, context: str, user: str)->None:
        self.send_queue.append([subject,context,user])
        
    def _send(self)->None:
        # smtpObj = smtplib.SMTP()
        subject,context,user = self.send_queue.pop(0)
        for i in range(5): # 最多尝试5次
            try:
                # smtpObj.connect('smtp.163.com', 25)
                smtpObj = smtplib.SMTP_SSL("smtp.exmail.qq.com", 465)
                smtpObj.login(self.sender, self.mail_pass)
                receiver = self.user_email[user]
                if receiver is None:
                    return
                message = MIMEText(context, 'plain', 'utf-8')
                message['Subject'] = subject
                message['From'] = '李乡儒团队GPU服务器'
                message['To'] = receiver
                smtpObj.sendmail(self.sender, receiver, message.as_string())
                smtpObj.quit()
                break
            except smtplib.SMTPException as e:
                print('邮件发送失败', e) 