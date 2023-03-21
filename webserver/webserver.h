#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QHttpServer>

class WebServer : public QObject
{
public:
    explicit WebServer(QObject *parent = nullptr);
    WebServer(int port,QString loginPage,QString userPage,QString staticPath);

signals:
    void a();


private:
    QHttpServer _server;
    QString _loginPage;
    QString _userPage;
    QString _staticPath;

    QMap<QString,QString>  getCookie(QString );
    QString  findHeaderItem(QList<QPair<QByteArray, QByteArray>> &,QString );
    QMap<QString,QString> toMap(QStringList &);
    bool testPasswd(QString,QString);
    QString user2md5(QString);
};

#endif // WEBSERVER_H
