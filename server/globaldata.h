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
#include "waitqueue.h"

class GlobalData : public QObject
{
    Q_OBJECT
public:
    static GlobalData& instance();
public:
    QHash<QString, QSharedPointer<QWebSocket>> wsClients;
    QObject* webServer;
    QWebSocketServer* wsServer;
    QTcpServer* tcpServer;
    QHash<QString, QSharedPointer<QTcpSocket>> tcpClients;
    QHash<QString, QJsonObject> gpus_cache;
    QJsonWebToken* jwt;
    WaitQueue* waitQueue;
    QTimer heartbeatTimer;

private:
    using QObject::QObject;
    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
};


