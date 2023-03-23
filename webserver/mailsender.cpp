#include "mailsender.h"

MailSender::MailSender() {
}

void MailSender::sendEmail(QString to, QString subject, QString body) {
    // 创建SMTP客户端对象
    QSslSocket *socket = new QSslSocket();
    socket->connectToHostEncrypted(smtpServer, smtpPort);

    if (socket->waitForConnected()) {
        // 读取服务器返回的欢迎信息
        if (socket->waitForReadyRead()) {
            socket->readAll();
        }

        // 发送EHLO命令
        socket->write("EHLO localhost\r\n");
        if (socket->waitForBytesWritten() && socket->waitForReadyRead()) {
            socket->readAll();
        }

        // 发送AUTH LOGIN命令
        socket->write("AUTH LOGIN\r\n");
        if (socket->waitForBytesWritten() && socket->waitForReadyRead()) {
            socket->readAll();
        }
        // 发送用户名和密码
        QByteArray login = smtpUser.toUtf8().toBase64();
        QByteArray password = smtpPassword.toUtf8().toBase64();
        socket->write(login + "\r\n");
        if (socket->waitForBytesWritten() && socket->waitForReadyRead()) {
            socket->readAll();
        }
        socket->write(password + "\r\n");
        if (socket->waitForBytesWritten() && socket->waitForReadyRead()) {
            socket->readAll();
        }

        // 发送MAIL FROM命令
        socket->write(("MAIL FROM:<" + from + ">\r\n").toUtf8());
        if (socket->waitForBytesWritten() && socket->waitForReadyRead()) {
            socket->readAll();
        }

        // 发送RCPT TO命令
        socket->write(("RCPT TO:<" + to + ">\r\n").toUtf8());
        if (socket->waitForBytesWritten() && socket->waitForReadyRead()) {
            socket->readAll();
        }

        // 发送DATA命令
        socket->write("DATA\r\n");
        if (socket->waitForBytesWritten() && socket->waitForReadyRead()) {
            socket->readAll();
        }

        // 发送邮件头和正文
        QString message = "From: " + from + "\r\n"
                          + "To: " + to + "\r\n"
                          + "Subject: " + subject + "\r\n"
                          + "\r\n"
                          + body + "\r\n"
                          + ".\r\n";
        socket->write(message.toUtf8());
        if (socket->waitForBytesWritten() && socket->waitForReadyRead()) {
            socket->readAll();
        }

        // 发送QUIT命令
        socket->write("QUIT\r\n");
        if (socket->waitForBytesWritten() && socket->waitForReadyRead()) {
            socket->readAll();
        }

        // 关闭连接
        socket->close();
    }
}

void MailSender::sendEmailWithUser(QString user, QString subject, QString body) {
    QString email = db->getEmail(user);
    if (email == "") {
        return;
    }
    sendEmail(email, subject, body);
}

void MailSender::sendTaskStatus(QString user, QString machine, QString container, QString status, QString ssType, QDateTime submitTime) {
    QString subject = "服务器作业" + status + "通知";
    QString body = "您于" + submitTime.toString() + "在服务器" + machine + (container == "" ? "" : ":" + container) + "上的作业已经" + status + "。";
    if (status == "容器内终止") {
        body += "你的作业已经提前完成，你可以在服务器上查看结果。";
    }
    if (status == "时间到期") {
        body += "你的作业时间结束,未结束的GPU进程已经被强制终止。";
    }
    if (status == "无指令启动") {
        body += "你的作业时间开始了，但是你没有提交任何指令，请尽快登陆容器进行作业。";
    }
    if (status == "指令启动") {
        body += "你的作业时间开始了，你托管的指令已经开始执行。";
    }
    this->sendEmailWithUser(user, subject, body);
}
