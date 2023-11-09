#pragma once

#include "../tools/jsonwebtoken/src/qjsonwebtoken.h"
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QSharedPointer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHttpServer>
#include <QTimer>
#include <QVariant>
#include <QWebSocket>
#include <QWebSocketServer>
#include "waitqueue.h"
#include "datastructure.hpp"
#include <QCoreApplication>
#include "database.h"


class GlobalData : public QObject
{
    Q_OBJECT
public:
    static GlobalData& instance();
public:
    QHttpServer httpServer;
    QWebSocketServer wsServer;
    QTcpServer tcpServer;
    WaitQueue waitQueue;
    QJsonWebToken jwt;
    DataBase database;


    QHash<QString, QSharedPointer<QWebSocket>> wsClients;
    QHash<QString, QSharedPointer<QTcpSocket>> tcpClients;
    QHash<QString, QString> gpus_cache;
    QHash<QString, Session> session_cache;
    QTimer heartbeatTimer;

private:
    GlobalData(QObject *parent = nullptr):
        QObject(qApp),
        httpServer(this),
        wsServer("", QWebSocketServer::NonSecureMode, this),
        tcpServer(this),
        waitQueue(this),
        heartbeatTimer(this)
    {};
    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
};


