#include "webserver.h"
#include <QMetaType>
#include <QSettings>
#include <QWeakPointer>
#include <QDebug>
using Method = QHttpServerRequest::Method;
WebServer::WebServer(QObject *parent)
    : QObject{parent}
{
    GlobalData::instance()->emplace("webserver", QVariant::fromValue(QSharedPointer<WebServer>(this)));
    _config = GlobalConfig::instance();
    _data = GlobalData::instance();
    _event = GlobalEvent::instance();
    _jwt = QSharedPointer<QJsonWebToken>::create();
    
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
}

WebServer::~WebServer()
{
}

QString WebServer::getJwtToken(const QString &identity) const
{
    _jwt->appendClaim("identity", identity);
    // _jwt->appendClaim("iat", QDateTime::currentDateTime().toSecsSinceEpoch());
    // _jwt->appendClaim("exp", QDateTime::currentDateTime().addDays(1).toSecsSinceEpoch());
    return _jwt->getToken();
}
template <typename T>
std::function<QHttpServerResponse (const QString &,const QHttpServerRequest &)> WebServer::jwtDecoratorArg(T t)
{
    return [this,&t](const QString &arg, const QHttpServerRequest &request)
    {
        auto header = request.headers();
        for (auto &item : header)
        {
            if (item.first == "Authorization")
            {
                auto jwt = QJsonWebToken::fromTokenAndSecret(item.second, this->_secret);
                if (jwt.isValid())
                    return std::invoke(t,arg,request);
                else
                {
                    QJsonObject res;
                    res["message"]="Invalid token";
                    return QHttpServerResponse(res, QHttpServerResponder::StatusCode::Unauthorized);
                }
            }
        }
        QJsonObject res;
        res["message"]="Token Not Found";
        return QHttpServerResponse(res, QHttpServerResponder::StatusCode::Unauthorized);
    };
}

template <typename T>
std::function<QHttpServerResponse(const QHttpServerRequest &)> WebServer::jwtDecorator(T t)
{
    return [this,&t](const QHttpServerRequest &request)
    {
        auto header = request.headers();
        for (auto &item : header)
        {
            if (item.first == "Authorization")
            {
                auto jwt = QJsonWebToken::fromTokenAndSecret(item.second, this->_secret);
                if (jwt.isValid())
                    return std::invoke(t,request);
                else
                {
                    QJsonObject res;
                    res["message"]="Invalid token";
                    return QHttpServerResponse(res, QHttpServerResponder::StatusCode::Unauthorized);
                }
            }
        }
        QJsonObject res;
        res["message"]="Token Not Found";
        return QHttpServerResponse(res, QHttpServerResponder::StatusCode::Unauthorized);
    };
}
