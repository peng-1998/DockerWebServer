#pragma once

#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <QVariant>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QTcpSocket>
#include <QTcpServer>
#include <QJsonObject>
#include "jsonwebtoken/src/qjsonwebtoken.h"
#include <QTimer>

class GlobalData : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalData> instance();
    QHash<QString, QSharedPointer<QWebSocket>> wsClients;
    QSharedPointer<QObject> webServer;
    QSharedPointer<QWebSocketServer> wsServer;
    QSharedPointer<QTcpServer> tcpServer;
    QHash<QString, QSharedPointer<QTcpSocket>> tcpClients;
    QHash<QString, QJsonObject> gpus_cache;
    QSharedPointer<QJsonWebToken> jwt;
    QTimer heartbeatTimer;
private:
    GlobalData(QObject *parent = nullptr);
    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
    static QSharedPointer<GlobalData> _instance;
};
