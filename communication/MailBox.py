import smtplib
from abc import ABC, abstractmethod
from email.mime.text import MIMEText
from email.utils import formataddr
from typing import Callable, List, Tuple


class BaseMail(ABC):

    def __init__(self, sender_mail: str, sender_name: str, password: str, resend_time: int = 5, logger: Callable = print) -> None:
        self.sender_mail = sender_mail
        self.sender_name = sender_name
        self.password    = password
        self.resend_time = resend_time
        self.logger      = logger
        self.send_queue: List[Tuple[str, str, str, str]] = []

    def append(self, receiver_mail: str, receiver_name: str, subject: str, content: str) -> None:
        self.send_queue.append((receiver_mail, receiver_name, subject, content))

    def send_all(self) -> None:
        for receiver_mail, receiver_name, subject, content in self.send_queue:
            for _ in range(self.resend_time):
                try:
                    self.send(receiver_mail, receiver_name, subject, content)
                    break
                except Exception as e:
                    self.logger(f'Failed to send mail to {receiver_mail} for {e}')
                    continue

    @abstractmethod
    def send(self, receiver_mail: str, receiver_name: str, subject: str, content: str) -> None:
        pass


class TencentEnterpriseMailbox(BaseMail):

    def __init__(self, sender_mail: str, sender_name: str, password: str, resend_time: int = 5, ssl: bool = True, logger: Callable = print) -> None:
        super().__init__(sender_mail, sender_name, password, resend_time, logger)
        self.smtp_server = 'smtp.exmail.qq.com'
        self.smtp_port = 465
        self.ssl = ssl

    def send(self, receiver_mail: str, receiver_name: str, subject: str, content: str) -> None:
        if self.ssl:
            server = smtplib.SMTP_SSL(self.smtp_server, self.smtp_port)
        else:
            server = smtplib.SMTP(self.smtp_server, self.smtp_port)
        server.login(self.sender_mail, self.password)
        msg = MIMEText(content, 'plain', 'utf-8')
        msg['From'] = formataddr([self.sender_name, self.sender_mail])
        msg['To'] = formataddr([receiver_name, receiver_mail])
        msg['Subject'] = subject
        server.sendmail(self.sender_mail, receiver_mail, msg.as_string())
        server.quit()


class GoogleMail(BaseMail):

    def send(self, receiver_mail: str, receiver_name: str, subject: str, content: str) -> None:
        smtp_server = 'smtp.gmail.com'
        smtp_port = 587

        # 使用SMTP_SSL连接服务器
        server = smtplib.SMTP(smtp_server, smtp_port)
        # 启用TLS加密
        server.ehlo()
        server.starttls()
        # 登录发件人邮箱
        server.login(self.sender_mail, self.password)

        # 构造邮件
        msg = MIMEText(content, 'plain', 'utf-8')
        msg['From'] = formataddr([self.sender_name, self.sender_mail])
        msg['To'] = formataddr([receiver_name, receiver_mail])
        msg['Subject'] = subject

        # 发送邮件
        server.sendmail(self.sender_mail, receiver_mail, msg.as_string())

        # 退出服务器
        server.quit()


class QQMail(BaseMail):

    def send(self, receiver_mail: str, receiver_name: str, subject: str, content: str) -> None:
        smtp_server = 'smtp.qq.com'
        smtp_port = 465

        # 使用SMTP_SSL连接服务器,QQ邮箱需要SSL加密
        server = smtplib.SMTP_SSL(smtp_server, smtp_port)
        # 登录发件人邮箱
        server.login(self.sender_mail, self.password)

        # 构造邮件
        msg = MIMEText(content, 'plain', 'utf-8')
        msg['From'] = formataddr([self.sender_name, self.sender_mail])
        msg['To'] = formataddr([receiver_name, receiver_mail])
        msg['Subject'] = subject

        # 发送邮件
        server.sendmail(self.sender_mail, receiver_mail, msg.as_string())

        # 退出服务器
        server.quit()