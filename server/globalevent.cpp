#include "globalevent.h"
#include "database.h"
#include "webserver.h"
#include <QFile>
#include <QJsonObject>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QDebug>
using StatusCode = QHttpServerResponder::StatusCode;


QSharedPointer<GlobalEvent> GlobalEvent::_instance = QSharedPointer<GlobalEvent>(new GlobalEvent());

QSharedPointer<GlobalEvent> GlobalEvent::instance()
{
    return _instance;
}

QHttpServerResponse GlobalEvent::onHttpIndex(const QHttpServerRequest &request)
{
    return QHttpServerResponse("Hello World!", StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onHttpWSServer(const QHttpServerRequest &request)
{
    int wsport = (*GlobalConfig::instance())["WebSocket"]["port"].as<int>();
    return QHttpServerResponse("{\"port\":" + QByteArray::number(wsport) + "}", StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onHttpWSClient(const QHttpServerRequest &request)
{
    int wsport = (*GlobalConfig::instance())["WebSocket"]["port"].as<int>();
    return QHttpServerResponse("{\"port\":" + QByteArray::number(wsport) + "}", StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiAuthLogin(const QHttpServerRequest &request)
{
    auto body = QJsonDocument::fromJson(request.body()).object();
    auto webserver = GlobalData::instance()->value("webserver").value<QSharedPointer<WebServer>>();
    auto username = body["username"].toString();
    auto db = DataBase::instance();
    if (!db->containsUser(username))
        return QHttpServerResponse({"message", "User Not Found"}, StatusCode::NotFound);
    auto user = db->getUser(username, QStringList() << "password"<< "salt").value();
    if (user["password"].toString() != GlobalCommon::hashPassword(body["password"].toString(), user["salt"].toString()))
        return QHttpServerResponse({"message", "Wrong Password"}, StatusCode::Unauthorized);
    return QHttpServerResponse({"access_token", webserver->getJwtToken(username)}, StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiAuthRegister(const QHttpServerRequest &request)
{
    auto db = DataBase::instance();
    auto body = QJsonDocument::fromJson(request.body()).object();
    auto username = body["username"].toString();
    auto password = body["password"].toString();
    if (db->containsUser(username))
        return QHttpServerResponse({"message", "User Already Exists"}, StatusCode::Conflict);
    auto [salt, hash] = GlobalCommon::generateSaltAndHash(password);
    db->insertUser(username, hash, salt, username, "", "", QString::fromStdString((*GlobalConfig::instance())["defaultPhoto"].as<std::string>()));
    return QHttpServerResponse(StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiUserSetProfile(const QHttpServerRequest &request)
{
    auto body = QJsonDocument::fromJson(request.body()).object();
    auto id = body["user_id"].toInt();
    auto db = DataBase::instance();
    db->updateUser(id, QList<QPair<QString, QVariant>>() << QPair<QString, QVariant>(body["field"].toString(), body["value"].toVariant()));
    return QHttpServerResponse(StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiUserSetPhoto(const QHttpServerRequest &request)
{
    auto headers = GlobalCommon::parseHeaders(request.headers());
    auto fileName = headers["filename"];
    auto account = headers["account"];
    auto id = headers["user_id"].toInt();
    auto withFile = headers["with_file"] == "true";
    auto db = DataBase::instance();

    if (!withFile)
        db->updateUser(id, QList<QPair<QString, QVariant>>() << QPair<QString, QVariant>("photo", QString::fromStdString((*GlobalConfig::instance())["defaultPhotoPath"].as<std::string>()) + "/" + fileName));
        return QHttpServerResponse(StatusCode::Ok);
    auto savePath = QString::fromStdString((*GlobalConfig::instance())["customPhotoPath"].as<std::string>()) + "/" + account + ".png";
    db->updateUser(id, QList<QPair<QString, QVariant>>() << QPair<QString, QVariant>("photo", savePath));
    QFile file(savePath);
    file.open(QIODevice::WriteOnly);
    file.write(request.body());
    file.close();
    return QHttpServerResponse(StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiUserGetUser(const QString &account, const QHttpServerRequest &request)
{
    auto result = DataBase::instance()->getUser(account);
    if(result.has_value())
        return QHttpServerResponse(StatusCode::NotFound);
    return QHttpServerResponse(GlobalCommon::hashToJsonObject(result.value()), StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiMachinesInfo(const QHttpServerRequest &request)
{
    auto machines = DataBase::instance()->getMachineAll();
    QJsonArray result;
    for (auto &machine : machines)
        result.append(GlobalCommon::hashToJsonObject(machine));
    return QHttpServerResponse(result, StatusCode::Ok);
}

GlobalEvent::GlobalEvent(QObject *parent)
    : QObject{parent}
{
}
