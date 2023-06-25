from abc import ABC, abstractmethod
import smtplib
from email.mime.text import MIMEText
from email.utils import formataddr
from typing import List, Tuple


class BaseMail(ABC):

    def __init__(self, sender_mail: str, sender_name: str, password: str) -> None:
        self.sender_mail = sender_mail
        self.sender_name = sender_name
        self.password = password
        self.send_queue: List[Tuple[str, str, str, str]] = []

    def append(self, receiver_mail: str, receiver_name: str, subject: str, content: str) -> None:
        self.send_queue.append((receiver_mail, receiver_name, subject, content))

    def send_all(self) -> None:
        for receiver_mail, receiver_name, subject, content in self.send_queue:
            self.send(receiver_mail, receiver_name, subject, content)

    @abstractmethod
    def send(self, receiver_mail: str, receiver_name: str, subject: str, content: str) -> None:
        pass


class TencentEnterpriseMailbox(BaseMail):

    def __init__(self, sender_mail: str, sender_name: str, password: str, smtp_server: str = 'smtp.exmail.qq.com', smtp_port: int = 465, ssl: bool = True) -> None:
        super().__init__(sender_mail, sender_name, password)
        self.smtp_server = smtp_server
        self.smtp_port = smtp_port
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

    def __init__(self, sender_mail: str, sender_name: str, password: str) -> None:
        super().__init__(sender_mail, sender_name, password)

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

    def __init__(self, sender_mail: str, sender_name: str, password: str) -> None:
        super().__init__(sender_mail, sender_name, password)

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