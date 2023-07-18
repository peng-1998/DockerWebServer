#pragma once

#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <QVariant>
#include <QWebSocket>
#include "webserver.h"
#include <QWebSocketServer>
#include <QTcpSocket>
#include <QTcpServer>
class GlobalData : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalData> instance();
    ~GlobalData();
    QHash<QString, QSharedPointer<QWebSocket>> wsClients;
    QSharedPointer<WebServer> webServer;
    QSharedPointer<QWebSocketServer> wsServer;
    QSharedPointer<QTcpServer> tcpServer;
    QHash<QString, QSharedPointer<QTcpSocket>> tcpClients;
    QHash<QString, QJsonObject> gpus_cache;

private:
    GlobalData();
    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
    static QSharedPointer<GlobalData> _instance;
};
