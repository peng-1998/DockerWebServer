from abc import ABC, abstractmethod
import smtplib
from email.mime.text import MIMEText
from email.utils import formataddr


class BaseMail(ABC):

    def __init__(self, sender_mail, sender_name, password):
        self.sender_mail = sender_mail
        self.sender_name = sender_name
        self.password = password
        self.send_queue = []

    def append(self, receiver_mail, receiver_name, subject, content):
        self.send_queue.append((receiver_mail, receiver_name, subject, content))

    def send_all(self):
        for receiver_mail, receiver_name, subject, content in self.send_queue:
            self.send(receiver_mail, receiver_name, subject, content)

    @abstractmethod
    def send(self, receiver_mail, receiver_name, subject, content):
        pass


class TencentEnterpriseMailbox(BaseMail):

    def __init__(self, sender_mail, sender_name, password, smtp_server='smtp.exmail.qq.com', smtp_port=465, ssl=True):
        super().__init__(sender_mail, sender_name, password)
        self.smtp_server = smtp_server
        self.smtp_port = smtp_port
        self.ssl = ssl

    def send(self, receiver_mail, receiver_name, subject, content):
        if self.ssl:
            server = smtplib.SMTP_SSL(self.smtp_server, self.smtp_port)
        else:
            server = smtplib.SMTP(self.smtp_server, self.smtp_port)
        server.login(self.sender_mail, self.password)
        msg = MIMEText(content, 'plain', 'utf-8')
        msg['From'] = formataddr([self.sender_name, self.sender_mail])
        msg['To'] = formataddr([receiver_name, receiver_mail])
        msg['Subject'] = subject
        server.sendmail(self.sender_mail, [receiver_mail, ], msg.as_string())
        server.quit()
