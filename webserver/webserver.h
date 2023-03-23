#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QHttpServer>
#include <QDateTime>
#include "machinepool.h"
#include "gpuwaitqueue.h"
#include "database.h"
class WebServer : public QObject {
public:
    explicit WebServer(QObject *parent = nullptr);
    WebServer(int port, QString loginPage, QString userPage, QString staticPath);

signals:
    void a();

private:
    QHttpServer _server;
    QString _loginPage;
    QString _userPage;
    QString _staticPath;
    MachinePool *_machinePool;
    TaskWaitQueue *_gpuWaitQueue;
    DataBase *_database;

    QMap<QString, QString> getCookie(QString);
    QString findHeaderItem(QList<QPair<QByteArray, QByteArray>> &, QString);
    QMap<QString, QString> toMap(QStringList &);
    bool checkIdentity(QList<QPair<QByteArray, QByteArray>> header);
    QString user2md5(QString);
    class Answer {
    public:
        Answer() {
            _answer = "";
            _lastupdate = QDateTime::currentDateTime().addDays(-1);
        }
        Answer(QString answer) {
            _answer = answer;
            _lastupdate = QDateTime::currentDateTime();
        }
        void setAnswer(const QString &newAnswer) {
            _answer = newAnswer;
            _lastupdate = QDateTime::currentDateTime();
        };
        QString answer() {
            return _answer;
        }
        QDateTime lastupdate() {
            return _lastupdate;
        }

    private:
        QString _answer;
        QDateTime _lastupdate;
    };

    QHash<QString, Answer> _answersCache;
    int _answerLive = 5;
};

#endif // WEBSERVER_H
