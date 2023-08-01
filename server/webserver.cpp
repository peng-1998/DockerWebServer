#include "webserver.h"
#include <QMetaType>
#include <QSettings>
#include <QWeakPointer>
#include <QDebug>

using Method = QHttpServerRequest::Method;
using StatusCode = QHttpServerResponder::StatusCode;

WebServer::WebServer(QObject *parent)
    : QObject{parent}
{
    GlobalConfig::init("config.yaml");
    _config = GlobalConfig::instance();
    _data   = GlobalData::instance();
    _event  = GlobalEvent::instance();
    _jwt    = QSharedPointer<QJsonWebToken>::create();
    GlobalData::instance()->tcpServer = QSharedPointer<QTcpServer>(new QTcpServer());
    GlobalData::instance()->tcpServer->listen(QHostAddress::Any, (*GlobalConfig::instance())["TCP"]["port"].as<int>());
    GlobalData::instance()->waitQueue = WaitQueue::instance();
    connect(GlobalData::instance()->tcpServer.get(), &QTcpServer::newConnection, GlobalEvent::instance().get(), &GlobalEvent::onNewTcpConnection);
    connect(&GlobalData::instance()->heartbeatTimer, &QTimer::timeout, GlobalEvent::instance().get(), &GlobalEvent::onCheckHeartbeat);
    GlobalData::instance()->heartbeatTimer.start(1000);
    _data->jwt = _jwt;
    
    _jwt->setSecret(QString::fromStdString((*_config)["JWT"]["secret"].as<std::string>()));
    _jwt->setAlgorithmStr(QString::fromStdString((*_config)["JWT"]["algorithm"].as<std::string>()));
    _jwt->appendClaim("iss", QString::fromStdString((*_config)["JWT"]["iss"].as<std::string>()));
    _httpServer = QSharedPointer<QHttpServer>::create();
    _httpServer->listen(QHostAddress::Any,(*_config)["Http"]["port"].as<int>());
    _httpServer->afterRequest([] (QHttpServerResponse &&resp) { 
        resp.addHeader("Access-Control-Allow-Origin", "*");
        return std::move(resp); 
    });
    _httpServer->route("/", Method::Get, &GlobalEvent::onHttpIndex);
    _httpServer->route("/ws/server", Method::Get, jwtDecorator(&GlobalEvent::onHttpWSServer));
    _httpServer->route("/ws/client", Method::Get, jwtDecorator(&GlobalEvent::onHttpWSClient));
    _httpServer->route("/api/auth/login", Method::Post, &GlobalEvent::onApiAuthLogin);
    _httpServer->route("/api/auth/register", Method::Post, &GlobalEvent::onApiAuthRegister);
    _httpServer->route("/api/user/set_profile", Method::Post, jwtDecorator(&GlobalEvent::onApiUserSetProfile));
    _httpServer->route("/api/user/set_photo", Method::Get, jwtDecorator(&GlobalEvent::onApiUserSetPhoto));
    _httpServer->route("/api/user/get_user/<arg>", Method::Get, jwtDecoratorArg(&GlobalEvent::onApiUserGetUser));
    _httpServer->route("/api/machines/info", Method::Get, jwtDecorator(&GlobalEvent::onApiMachinesInfo));
    _httpServer->route("/api/admin/all_users", Method::Get, jwtDecorator(&GlobalEvent::onApiAdminAllUsers));
    _httpServer->route("/api/admin/all_images", Method::Get, jwtDecorator(&GlobalEvent::onApiAdminAllImages));
    _httpServer->route("/api/admin/all_containers/<arg>", Method::Get, jwtDecoratorArg(&GlobalEvent::onApiAdminAllContainers));
    _httpServer->route("/api/task/cancel", Method::Post, jwtDecorator(&GlobalEvent::onApiTaskCancel));

    _wsServer = QSharedPointer<QWebSocketServer>::create("WebSocketServer", QWebSocketServer::NonSecureMode);
    _wsServer->listen(QHostAddress::Any, (*_config)["WebSocket"]["port"].as<int>());
    GlobalData::instance()->wsServer = _wsServer;
    connect(_wsServer.get(), &QWebSocketServer::newConnection, _event.get(), &GlobalEvent::onWSNewConnection);
}

WebServer::~WebServer()
{
}

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
