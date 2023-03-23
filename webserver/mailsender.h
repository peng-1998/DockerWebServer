#ifndef MAILSENDER_H
#define MAILSENDER_H

#include <QObject>
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSsl>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include "database.h"
class MailSender : public QObject
{
    Q_OBJECT
public:
    MailSender();
    void sendEmail(QString to, QString subject, QString body);
    void sendEmailWithUser(QString user, QString subject, QString body);
    void sendTaskStatus(QString user, QString machine, QString container, QString status, QString ssType, QDateTime submitTime);
    
private:
    // SMTP服务器信息
    QString smtpServer = "smtp.example.com"; // SMTP服务器地址
    int smtpPort = 465;                      // SMTP服务器端口号
    QString smtpUser = "your_username";      // SMTP用户名
    QString smtpPassword = "your_password";  // SMTP密码
    QString from = "your_email@example.com"; // 发件人邮箱地址
    DataBase *db;

signals:


};

#endif // MAILSENDER_H

