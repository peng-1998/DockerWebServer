#pragma once

#include "../tools/globalconfig.h"
#include "../tools/jsonwebtoken/src/qjsonwebtoken.h"
#include "globalcommon.h"
#include "waitqueue.h"
#include <HttpReq>
#include <QHttpServerResponse>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QTimer>
#include <QWeakPointer>
#include <QWebSocket>


#define request_ const HttpReq &request
#define session_ const QString &session
class GlobalEvent : public QObject
{
    Q_OBJECT
    using HttpRep = QHttpServerResponse;
    using HttpReq = QHttpServerrequest;
public:
    static GlobalEvent &instance();
public slots:
    // http handlers
    // static HttpRep onHttpIndex(request_);
    // static HttpRep onHttpWSServer(request_);
    // static HttpRep onHttpWSClient(request_);

    static HttpRep onApiAuthLogin(request_);
    static HttpRep onApiAuthRegister(request_);
    static HttpRep onApiAuthLogout(request_, session_);
    static HttpRep onApiAuthsession(request_, session_);
    static HttpRep onApiUserSetProfile(request_, session_);
    static HttpRep onApiUserSetPhoto(request_, session_);
    static HttpRep onApiUserGetUser(request_, session_);
    static HttpRep onApiMachinesInfo(request_, session_);
    static HttpRep onApiAdminAllUsers(request_, session_);
    static HttpRep onApiAdminAllImages(request_, session_);
    static HttpRep onApiAdminAllContainers(const QString &machineId, request_, session_);
    static HttpRep onApiTaskCancel(request_, session_);
    static HttpRep onApiTaskrequest_(request_, session_);
    static HttpRep onApiTaskUser(const QString &account, request_, session_);
    static HttpRep onApiTaskMachine(const QString &machineId, request_, session_);
    static HttpRep onApiAdminAllTasks(request_, session_);

    auto GlobalEvent::sessionDecorator(auto &&f)
    {
        return [std::forward<decltype(f)>(f)](auto &&...args, request_, session_) -> HttpRep
        {
            auto header = request_.headers();
            for (auto &item : header)
                if (item.first == "Authorization")
                    if (
                        auto jwt = QJsonWebToken::fromTokenAndSecret(
                            item.second, GlobalData::instance().jwt.getSecret());
                        jwt.isValid())
                    {
                        if (!GlobalData::instance().session_cache.contains(item.second))
                        {
                            auto account = jwt.getClaim("identity").toString();
                            GlobalData::instance().session_cache.insert(
                                item.second,
                                account);
                        }
                        return std::invoke(f, std::forward<decltype(args)>(args)..., request, item.second);
                    }
                    else
                        return HttpRep(QJsonObject{{"message", "Invalid token"}}, StatusCode::Unauthorized);
            return HttpRep(QJsonObject{{"message", "Token Not Found"}}, StatusCode::Unauthorized);
        };
    }

    void onWSNewConnection();
    void onWSDisconnection(const QString &uuid);
    void onWSMessageReceived(const QString &message, const QString &uuid);
    void onWSHandleContainer(const QJsonObject &data, const QString &uuid);
    void onWSHandleImage(const QJsonObject &data, const QString &uuid);
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
    QHash<QString, std::function<void(const QJsonObject &, const QString &)>> _wsHandlers;
    QHash<QString, std::function<void(const QJsonObject &, const QString &)>> _tcpHandlers;
};

#undef request_
#undef session_