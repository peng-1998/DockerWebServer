#pragma once

#include "../tools/jsonwebtoken/src/qjsonwebtoken.h"
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QSharedPointer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QVariant>
#include <QWebSocket>
#include <QWebSocketServer>

class GlobalData : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalData> instance();
public:
    QHash<QString, QSharedPointer<QWebSocket>> wsClients;
    QSharedPointer<QObject> webServer;
    QSharedPointer<QWebSocketServer> wsServer;
    QSharedPointer<QTcpServer> tcpServer;
    QHash<QString, QSharedPointer<QTcpSocket>> tcpClients;
    QHash<QString, QJsonObject> gpus_cache;
    QSharedPointer<QJsonWebToken> jwt;
    QTimer heartbeatTimer;

private:
    using QObject::QObject;
    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
    static QSharedPointer<GlobalData> _instance;


};
