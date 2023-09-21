#include "webserver.h"
#include <QMetaType>
#include <QSettings>
#include <QWeakPointer>
#include <QDebug>

using Method = QHttpServerRequest::Method;
using StatusCode = QHttpServerResponder::StatusCode;

WebServer::WebServer(QObject *parent)
    : QObject{parent}, _tcpServer{this}, _httpServer{this}
{
    GlobalConfig::instance().init("config.yaml");
    
    _jwt = QJsonWebToken{};

    GlobalData::instance().tcpServer = &_tcpServer;
    _tcpServer.listen(QHostAddress::Any, GlobalConfig::instance()["TCP"]["port"].as<int>());

    GlobalData::instance().waitQueue = &WaitQueue::instance();
    connect(GlobalData::instance().tcpServer, &QTcpServer::newConnection, &GlobalEvent::instance(), &GlobalEvent::onNewTcpConnection);
    connect(&GlobalData::instance().heartbeatTimer, &QTimer::timeout, &GlobalEvent::instance(), &GlobalEvent::onCheckHeartbeat);
    GlobalData::instance().heartbeatTimer.start(1000);
    GlobalData::instance().jwt = &_jwt;
    
    _jwt.setSecret(QString::fromStdString(GlobalConfig::instance()["JWT"]["secret"].as<std::string>()));
    _jwt.setAlgorithmStr(QString::fromStdString(GlobalConfig::instance()["JWT"]["algorithm"].as<std::string>()));
    _jwt.appendClaim("iss", QString::fromStdString(GlobalConfig::instance()["JWT"]["iss"].as<std::string>()));

    _httpServer.listen(QHostAddress::Any,GlobalConfig::instance()["Http"]["port"].as<int>());
    _httpServer.afterRequest([] (QHttpServerResponse &&resp) { 
        resp.addHeader("Access-Control-Allow-Origin", "*");
        return std::move(resp);
    });
    _httpServer.route("/", Method::Get, &GlobalEvent::onHttpIndex);
    _httpServer.route("/ws/server", Method::Get, jwtDecorator(&GlobalEvent::onHttpWSServer));
    _httpServer.route("/ws/client", Method::Get, jwtDecorator(&GlobalEvent::onHttpWSClient));
    _httpServer.route("/api/auth/login", Method::Post, &GlobalEvent::onApiAuthLogin);
    _httpServer.route("/api/auth/register", Method::Post, &GlobalEvent::onApiAuthRegister);
    _httpServer.route("/api/user/set_profile", Method::Post, jwtDecorator(&GlobalEvent::onApiUserSetProfile));
    _httpServer.route("/api/user/set_photo", Method::Get, jwtDecorator(&GlobalEvent::onApiUserSetPhoto));
    _httpServer.route("/api/user/get_user/<arg>", Method::Get, jwtDecoratorArg(&GlobalEvent::onApiUserGetUser));
    _httpServer.route("/api/machines/info", Method::Get, jwtDecorator(&GlobalEvent::onApiMachinesInfo));
    _httpServer.route("/api/admin/all_users", Method::Get, jwtDecorator(&GlobalEvent::onApiAdminAllUsers));
    _httpServer.route("/api/admin/all_images", Method::Get, jwtDecorator(&GlobalEvent::onApiAdminAllImages));
    _httpServer.route("/api/admin/all_containers/<arg>", Method::Get, jwtDecoratorArg(&GlobalEvent::onApiAdminAllContainers));
    _httpServer.route("/api/task/cancel", Method::Post, jwtDecorator(&GlobalEvent::onApiTaskCancel));

    _wsServer = new QWebSocketServer("WebSocketServer", QWebSocketServer::NonSecureMode);
    _wsServer->listen(QHostAddress::Any, GlobalConfig::instance()["WebSocket"]["port"].as<int>());
    GlobalData::instance().wsServer = _wsServer;
    connect(_wsServer, &QWebSocketServer::newConnection, &GlobalEvent::instance(), &GlobalEvent::onWSNewConnection);
}

// template <typename Func>
// auto WebServer::jwtDecoratorArg(Func && f)
// {
//     return [this,std::forward<Func>(f)](auto && ...args,auto request)->auto
//     {
//         auto header = request.headers();
//         for (auto &item : header)
//             if (item.first == "Authorization")
//                 if (auto jwt = QJsonWebToken::fromTokenAndSecret(item.second, this->_secret); jwt.isValid())
//                     return std::invoke(f, std::forward<decltype(args)>(args)..., request);
//                 else
//                     return QHttpServerResponse(QJsonObject{{"message","Invalid token"}}, StatusCode::Unauthorized);
//         return QHttpServerResponse(QJsonObject{{"message","Token Not Found"}}, StatusCode::Unauthorized);
//     };
// };



template <typename T>
std::function<QHttpServerResponse (QString ,const QHttpServerRequest &)> WebServer::jwtDecoratorArg(T && t)
{
    return [this, t = std::forward<T>(t)](QString arg, const QHttpServerRequest &request) -> QHttpServerResponse
    {
        auto header = request.headers();
        for (auto &item : header)
            if (item.first == "Authorization")
                if (auto jwt = QJsonWebToken::fromTokenAndSecret(item.second, this->_secret); jwt.isValid())
                    return std::invoke(t, arg, request);
                else
                    return QHttpServerResponse(QJsonObject{{"message","Invalid token"}}, StatusCode::Unauthorized);
        return QHttpServerResponse(QJsonObject{{"message","Token Not Found"}}, StatusCode::Unauthorized);
    };
}

template <typename T>
std::function<QHttpServerResponse(const QHttpServerRequest &)> WebServer::jwtDecorator(T && t)
{
    return [this, t = std::forward<T>(t)](const QHttpServerRequest &request) -> QHttpServerResponse
    {
        auto header = request.headers();
        for (auto &item : header)
            if (item.first == "Authorization")
                if (auto jwt = QJsonWebToken::fromTokenAndSecret(item.second, this->_secret); jwt.isValid())
                    return std::invoke(t, request);
                else
                    return QHttpServerResponse(QJsonObject{{"message","Invalid token"}}, StatusCode::Unauthorized);
        return QHttpServerResponse(QJsonObject{{"message","Token Not Found"}}, StatusCode::Unauthorized);
    };
}
