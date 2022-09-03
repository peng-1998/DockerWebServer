import smtplib
from email.mime.text import MIMEText
import time


class EmailMessager:

    def __init__(self, mail_login_user: str, mail_password: str, mail_sender: str, user_email: dict, from_str: str, server_host: str, server_port: int, use_ssl: bool = False, max_resend_time: int = 5) -> None:
        self.mail_login_user = mail_login_user
        self.mail_password = mail_password
        self.mail_sender = mail_sender
        self.user_email = user_email
        self.from_str = from_str
        self.server_host = server_host
        self.server_port = server_port
        self.use_ssl = use_ssl
        self.max_resend_time = max_resend_time
        self.send_queue = []

    def send(self, subject: str, context: str, user: str) -> None:
        self.send_queue.append([subject, context, user])

    def _send(self) -> None:
        subject, context, user = self.send_queue.pop(0)
        for i in range(self.max_resend_time):
            try:
                if self.use_ssl:
                    server = smtplib.SMTP_SSL(self.server_host, self.server_port)
                else:
                    server = smtplib.SMTP(self.server_host, self.server_port)
                server.login(self.mail_login_user, self.mail_password)
                receiver = self.user_email[user]
                msg = MIMEText(context, 'plain', 'utf-8')
                msg['Subject'] = subject
                msg['From'] = self.from_str
                msg['To'] = receiver
                server.sendmail(self.mail_sender, receiver, msg.as_string())
                server.quit()
                break
            except smtplib.SMTPException as e:
                print('邮件发送失败', e)
                time.sleep(1)
