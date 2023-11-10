#pragma once

#include "../tools/globalconfig.hpp"
#include "../tools/jsonwebtoken/src/qjsonwebtoken.h"
#include "../tools/logger.hpp"
#include "database.h"
#include "datastructure.hpp"
#include "waitqueue.h"
#include <QCoreApplication>
#include <QHash>
#include <QHttpServer>
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
    static GlobalData &instance()
    {
        static GlobalData _instance;
        return _instance;
    }

public:
    QHttpServer httpServer;
    QWebSocketServer wsServer;
    QTcpServer tcpServer;
    WaitQueue waitQueue;
    QJsonWebToken jwt;
    DataBase database;
    Logger logger;
    QTimer heartbeatTimer;

    QHash<QString, QSharedPointer<QWebSocket>> wsClients;
    QHash<QString, QSharedPointer<QTcpSocket>> tcpClients;
    QHash<QString, QString> gpus_cache;
    QHash<QString, Session> session_cache;

private:
    GlobalData(QObject *parent = nullptr) : QObject(qApp),
                                            httpServer(this),
                                            wsServer("", QWebSocketServer::NonSecureMode, this),
                                            tcpServer(this),
                                            waitQueue(this),
                                            heartbeatTimer(this),
                                            logger(QString::fromStdString(GlobalConfig::instance()["logPath"].as<std::string>())){};
    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
};
