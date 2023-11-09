#pragma once

#include "../tools/globalconfig.h"
#include "../tools/jsonwebtoken/src/qjsonwebtoken.h"
#include "database.h"
#include "globalcommon.h"
#include "globaldata.h"
#include "waitqueue.h"
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
#include <functional>

#define request_ const HttpReq &request
#define session_ const QString &session
class GlobalEvent : public QObject
{
    Q_OBJECT
    using HttpRep = QHttpServerResponse;
    using HttpReq = QHttpServerRequest;

public:
    static GlobalEvent &instance();
    auto sessionDecorator(std::function<HttpRep(const QString &, const HttpReq &, const QString &)> &&f)
    {
        return [f = std::forward<decltype(f)>(f)](const QString &args, request_) -> HttpRep
        {
            using namespace std;
            using StatusCode = QHttpServerResponse::StatusCode;
            static auto *const globalData_ptr = &GlobalData::instance();
            static auto *const session_cache_ptr = &(globalData_ptr->session_cache);
            static auto *const database_ptr = &(globalData_ptr->database);
            auto header = request.headers();
            for (auto &item : header)
                if (item.first == "Authorization")
                    if (
                        auto jwt = QJsonWebToken::fromTokenAndSecret(
                            item.second, globalData_ptr->jwt.getSecret());
                        jwt.isValid())
                    {
                        if (!session_cache_ptr->contains(item.second))
                        {
                            auto account = jwt.claim("identity");
                            auto userId_ = database_ptr->getUser(account, {{{"id"}}});
                            if (userId_.has_value())
                            {
                                uint userId = userId_.value()["id"].toInt();
                                session_cache_ptr->insert(
                                    QString(item.second),
                                    {userId, account});
                            }
                            else
                                return HttpRep(QJsonObject{{"message", "User not include"}}, StatusCode::Unauthorized);
                        }
                        return std::invoke(f, std::forward<decltype(args)>(args), request, item.second);
                    }
                    else
                        return HttpRep(QJsonObject{{"message", "Invalid token"}}, StatusCode::Unauthorized);
            return HttpRep(QJsonObject{{"message", "Token Not Found"}}, StatusCode::Unauthorized);
        };
    };

    auto sessionDecorator(std::function<HttpRep(const HttpReq &, const QString &)> &&f)
    {
        return [f = std::forward<decltype(f)>(f)](request_) -> HttpRep
        {
            using namespace std;
            using StatusCode = QHttpServerResponse::StatusCode;
            static auto *const globalData_ptr = &GlobalData::instance();
            static auto *const session_cache_ptr = &(globalData_ptr->session_cache);
            static auto *const database_ptr = &(globalData_ptr->database);
            auto header = request.headers();
            for (auto &item : header)
                if (item.first == "Authorization")
                    if (
                        auto jwt = QJsonWebToken::fromTokenAndSecret(
                            item.second, globalData_ptr->jwt.getSecret());
                        jwt.isValid())
                    {
                        if (!session_cache_ptr->contains(item.second))
                        {
                            auto account = jwt.claim("identity");
                            auto userId_ = database_ptr->getUser(account, {{{"id"}}});
                            if (userId_.has_value())
                            {
                                uint userId = userId_.value()["id"].toInt();
                                session_cache_ptr->insert(
                                    QString(item.second),
                                    {userId, account});
                            }
                            else
                                return HttpRep(QJsonObject{{"message", "User not include"}}, StatusCode::Unauthorized);
                        }
                        return std::invoke(f, request, item.second);
                    }
                    else
                        return HttpRep(QJsonObject{{"message", "Invalid token"}}, StatusCode::Unauthorized);
            return HttpRep(QJsonObject{{"message", "Token Not Found"}}, StatusCode::Unauthorized);
        };
    };
public slots:
    // http handlers
    // static HttpRep onHttpIndex(request_);
    // static HttpRep onHttpWSServer(request_);
    // static HttpRep onHttpWSClient(request_);

    static HttpRep onApiAuthLogin(request_);
    static HttpRep onApiAuthRegister(request_);
    static HttpRep onApiAuthLogout(request_, session_);
    static HttpRep onApiAuthSession(request_, session_);
    static HttpRep onApiUserSetProfile(request_, session_);
    static HttpRep onApiUserSetPhoto(request_, session_);
    static HttpRep onApiUserGetUser(request_, session_);
    static HttpRep onApiMachinesInfo(request_, session_);
    static HttpRep onApiAdminAllUsers(request_, session_);
    static HttpRep onApiAdminAllImages(request_, session_);
    static HttpRep onApiAdminAllContainers(const QString &machineId, request_, session_);
    static HttpRep onApiTaskCancel(request_, session_);
    static HttpRep onApiTaskRequest(request_, session_);
    static HttpRep onApiTaskUser(const QString &account, request_, session_);
    static HttpRep onApiTaskMachine(const QString &machineId, request_, session_);
    static HttpRep onApiAdminAllTasks(request_, session_);

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