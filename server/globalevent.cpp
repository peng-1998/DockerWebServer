#include "globalevent.h"
#include "database.h"
#include "webserver.h"
#include <QJsonObject>
#include <QWebSocket>
#include <QWebSocketServer>
using StatusCode = QHttpServerResponder::StatusCode;
QSharedPointer<GlobalEvent> GlobalEvent::_instance = QSharedPointer<GlobalEvent>::create();

QSharedPointer<GlobalEvent> GlobalEvent::instance()
{
    return _instance;
}

QHttpServerResponse GlobalEvent::onHttpWSServer(const QHttpServerRequest &request)
{
    int wsport = GlobalConfig::instance()->value("WebSocket/port").toInt();
    return QHttpServerResponse("{\"port\":" + QByteArray::number(wsport) + "}", StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onHttpWSClient(const QHttpServerRequest &request)
{
    int wsport = GlobalConfig::instance()->value("WebSocket/port").toInt();
    return QHttpServerResponse("{\"port\":" + QByteArray::number(wsport) + "}", StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiAuthLogin(const QHttpServerRequest &request)
{
    auto headers = GlobalCommon::parseHeaders(request.headers());
    auto webserver = GlobalData::instance()->value("webserver").value<QSharedPointer<WebServer>>();
    auto username = headers["username"];
    auto db = DataBase::instance();
    auto res = QJsonObject();
    if (!db->containsUser(username))
    {
        res["success"] = false;
        res["message"] = "User Not Found";
        return QHttpServerResponse(res, StatusCode::NotFound);
    }
    auto user = db->getUser(username, QStringList() << "password"
                                                    << "salt")
                    .value();
    if (user["password"].toString() != GlobalCommon::hashPassword(headers["password"], user["salt"].toString()))
    {
        res["success"] = false;
        res["message"] = "Wrong Password";
        return QHttpServerResponse(res, StatusCode::Unauthorized);
    }
    res["success"] = true;
    res["message"] = "Login Succeed";
    res["access_token"] = webserver->getJwtToken(username);
    return QHttpServerResponse(res, StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiAuthRegister(const QHttpServerRequest &request)
{
    auto db = DataBase::instance();
    auto headers = GlobalCommon::parseHeaders(request.headers());
    auto username = headers["username"];
    auto password = headers["password"];
    if (db->containsUser(username))
    {
        QJsonObject res;
        res["success"] = false;
        res["message"] = "User Already Exists";
        return QHttpServerResponse(res, StatusCode::Conflict);
    }
    auto [salt, hash] = GlobalCommon::generateSaltAndHash(password);
    db->insertUser(username, hash, salt);
}

GlobalEvent::GlobalEvent(QObject *parent)
    : QObject{parent}
{
}
