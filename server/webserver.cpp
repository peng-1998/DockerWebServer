#include "webserver.h"
#include <QMetaType>
#include <QSettings>
#include <QWeakPointer>
//
using Method = QHttpServerRequest::Method;
WebServer::WebServer(QObject *parent)
    : QObject{parent}
{
    GlobalConfig::init("config.ini", QSettings::IniFormat);
    GlobalData::instance()->emplace("webserver", QVariant::fromValue(QSharedPointer<WebServer>(this)));
    _config = GlobalConfig::instance();
    _common = GlobalCommon::instance();
    _data = GlobalData::instance();
    _event = GlobalEvent::instance();
    _jwt = QSharedPointer<QJsonWebToken>::create();
    _jwt->setSecret(_config->value("JWT/secret").toString());
    _jwt->setAlgorithmStr(_config->value("JWT/algorithm").toString());
    _jwt->appendClaim("iss", _config->value("JWT/iss").toString());
    auto bind = [this](auto func)
    { return std::bind(func, this->_event, std::placeholders::_1); };
    _httpServer = QSharedPointer<QHttpServer>::create();
    _httpServer->listen(QHostAddress::Any, _config->value("Http/port").toInt());
    _httpServer->route("/", Method::Get, bind(&GlobalEvent::onHttpIndex));
    _httpServer->route("/ws/server", Method::Get, jwtDecorator(GlobalEvent::onHttpWSServer));
    _httpServer->route("/ws/client", Method::Get, jwtDecorator(GlobalEvent::onHttpWSClient));
    _httpServer->route("/api/auth/login", Method::Post, bind(&GlobalEvent::onApiAuthLogin));
    _httpServer->route("/api/auth/register", Method::Post, bind(&GlobalEvent::onApiAuthRegister));
    _httpServer->route("/api/user/set_profile", Method::Post, jwtDecorator(GlobalEvent::onApiUserSetProfile));
    _httpServer->route("/api/user/set_photo", Method::Get, jwtDecorator(GlobalEvent::onApiUserSetPhoto));
    _httpServer->route("/api/user/get_user/<arg>", Method::Get, jwtDecorator(GlobalEvent::onApiUserGetUser));
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
std::function<QHttpServerResponse(const QHttpServerRequest &)> WebServer::jwtDecoratorArg(T t)
{
    return [this](const QString &account, const QHttpServerRequest &request) -> QHttpServerResponse
    {
        auto header = request.headers();
        for (auto item : header)
        {
            if (item.first == "Authorization")
            {
                auto token = item.second;
                auto jwt = QJsonWebToken::fromTokenAndSecret(token, this->_secret);
                if (jwt->isValid())
                    return t(account, request);
                else
                    return QHttpServerResponse({"message", "Invalid token"}, QHttpServerResponder::StatusCode::Unauthorized);
            }
        }
    };
}

template <typename T>
inline std::function<QHttpServerResponse(const QHttpServerRequest &)> WebServer::jwtDecorator(T t)
{
    return [this](const QHttpServerRequest &request) -> QHttpServerResponse
    {
        auto header = request.headers();
        for (auto item : header)
        {
            if (item.first == "Authorization")
            {
                auto token = item.second;
                auto jwt = QJsonWebToken::fromTokenAndSecret(token, this->_secret);
                if (jwt->isValid())
                    return t(request);
                else
                    return QHttpServerResponse({"message", "Invalid token"}, QHttpServerResponder::StatusCode::Unauthorized);
            }
        }
    };
}