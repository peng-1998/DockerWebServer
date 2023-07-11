#include "globalevent.h"
#include <QJsonObject>
#include <QWebSocket>
#include <QWebSocketServer>
#include "database.h"
using StatusCode = QHttpServerResponder::StatusCode;
QSharedPointer<GlobalEvent> GlobalEvent::_instance = QSharedPointer<GlobalEvent>::create();

QSharedPointer<GlobalEvent> GlobalEvent::instance()
{
    return _instance;
}

QHttpServerResponse &&GlobalEvent::onHttpWSServer(const QHttpServerRequest &request)
{
    int wsport = GlobalConfig::instance()->value("WebSocket/por+t").toInt();
    return QHttpServerResponse("{\"port\":" + QByteArray::number(wsport) + "}", StatusCode::Ok);
}

QHttpServerResponse &&GlobalEvent::onHttpWSClient(const QHttpServerRequest &request)
{
    int wsport = GlobalConfig::instance()->value("WebSocket/por+t").toInt();
    return QHttpServerResponse("{\"port\":" + QByteArray::number(wsport) + "}", StatusCode::Ok);
}

QHttpServerResponse &&GlobalEvent::onApiAuthLogin(const QHttpServerRequest &request)
{
    auto headers = GlobalCommon::parseHeaders(request.headers());
    auto username = headers["username"];
    auto db = DataBase::instance();
    if(!db->containsUser(username))
    {
        auto res = QJsonObject();
        res["success"] = false;
        res["message"] = "User Not Found";
        return QHttpServerResponse(res, StatusCode::Ok);
    }
    auto password = headers["password"];
    
    return QHttpServerResponse(StatusCode::Ok);
}

GlobalEvent::GlobalEvent(QObject *parent)
    : QObject{parent}
{
}
