#pragma once

#include "globalcommon.h"
#include "../tools/globalconfig.h"
#include "../tools/jsonwebtoken/src/qjsonwebtoken.h"
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QTimer>
#include <QWeakPointer>
#include <QWebSocket>
class GlobalEvent : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalEvent> instance();
public slots:
    static QHttpServerResponse onHttpIndex(const QHttpServerRequest &request);
    static QHttpServerResponse onHttpWSServer(const QHttpServerRequest &request);
    static QHttpServerResponse onHttpWSClient(const QHttpServerRequest &request);
    static QHttpServerResponse onApiAuthLogin(const QHttpServerRequest &request);
    static QHttpServerResponse onApiAuthRegister(const QHttpServerRequest &request);
    static QHttpServerResponse onApiUserSetProfile(const QHttpServerRequest &request);
    static QHttpServerResponse onApiUserSetPhoto(const QHttpServerRequest &request);
    static QHttpServerResponse onApiUserGetUser(const QString &account, const QHttpServerRequest &request);
    static QHttpServerResponse onApiMachinesInfo(const QHttpServerRequest &request);
    static QHttpServerResponse onApiAdminAllUsers(const QHttpServerRequest &request);
    static QHttpServerResponse onApiAdminAllImages(const QHttpServerRequest &request);
    static QHttpServerResponse onApiAdminAllContainers(const QString &machineId, const QHttpServerRequest &request);
    void onWSNewConnection();
    void onWSDisconnection(const QString &uuid);
    void onWSMessageReceived(const QString &message, const QString &uuid);
    void onWSHandleContainer(QJsonObject &data, const QString &uuid);
    void onNewTcpConnection();
    void onTcpMessageReceived();
    void onTcpDisconnection(const QString &machineId);
    void onTcpHandleInit(QJsonObject &data, QTcpSocket *sder);
    void onTcpHandleContainer(QJsonObject &data, const QString &machineId);
    void onTcpHandleGpus(QJsonObject &data, const QString &machineId);
    void onTcpHandleImage(QJsonObject &data, const QString &machineId);
    void onTcpHandleHeartbeat(QJsonObject &data, const QString &machineId);
    void onCheckHeartbeat();
signals:

private:
    explicit GlobalEvent(QObject *parent = nullptr);
    GlobalEvent(const GlobalEvent &) = delete;
    GlobalEvent &operator=(const GlobalEvent &) = delete;
    GlobalEvent(GlobalEvent &&) = delete;
    static QSharedPointer<GlobalEvent> _instance;
    QHash<QString, std::function<void(GlobalEvent *, QJsonObject &data, const QString &uuid)>> _wsHandlers;
    QHash<QString, std::function<void(GlobalEvent *, QJsonObject &data, const QString &machineId)>> _tcpHandlers;
};
