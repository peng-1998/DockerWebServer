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
#include "waitqueue.h"
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
    static QHttpServerResponse onApiTaskCancel(const QHttpServerRequest &request);
    static QHttpServerResponse onApiTaskRequest(const QHttpServerRequest &request);
    static QHttpServerResponse onApiTaskUser(const QString &account, const QHttpServerRequest &request);
    static QHttpServerResponse onApiTaskMachine(const QString &machineId, const QHttpServerRequest &request);
    static QHttpServerResponse onApiAdminAllTasks(const QHttpServerRequest &request);
    void onWSNewConnection();
    void onWSDisconnection(const QString &uuid);
    void onWSMessageReceived(const QString &message, const QString &uuid);
    void onWSHandleContainer(const QJsonObject &data, const QString &uuid);
    void onNewTcpConnection();
    void onTcpMessageReceived();
    void onTcpDisconnection(const QString &machineId);
    void onTcpHandleInit(const QJsonObject &data, QTcpSocket *sder);
    void onTcpHandleContainer(const QJsonObject &data, const QString &machineId);
    void onTcpHandleGpus(const QJsonObject &data, const QString &machineId);
    void onTcpHandleImage(const QJsonObject &data, const QString &machineId);
    void onTcpHandleHeartbeat(const QJsonObject &data, const QString &machineId);
    void onCheckHeartbeat();
    void onRunTask(Task task);
    void onTaskTimeout(quint64 taskId, const QString &machineId);

signals:

private:
    explicit GlobalEvent(QObject *parent = nullptr);
    GlobalEvent(const GlobalEvent &) = delete;
    GlobalEvent &operator=(const GlobalEvent &) = delete;
    GlobalEvent(GlobalEvent &&) = delete;
    static QSharedPointer<GlobalEvent> _instance;
    QHash<QString, std::function<void(const QJsonObject &, const QString &)>> _wsHandlers;
    QHash<QString, std::function<void(const QJsonObject &, const QString &)>> _tcpHandlers;
};
