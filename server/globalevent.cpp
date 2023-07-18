#include "globalevent.h"
#include "database.h"
#include "webserver.h"
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QUuid>
#include <QWebSocket>
#include <QWebSocketServer>

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
    auto body      = QJsonDocument::fromJson(request.body()).object();
    auto webserver = GlobalData::instance()->webServer;
    auto username  = body["username"].toString();
    auto db        = DataBase::instance();
    if (!db->containsUser(username))
        return QHttpServerResponse({"message", "User Not Found"}, StatusCode::NotFound);
    auto user = db->getUser(username, QStringList() << "password"
                                                    << "salt")
                    .value();
    if (user["password"].toString() != GlobalCommon::hashPassword(body["password"].toString(), user["salt"].toString()))
        return QHttpServerResponse({"message", "Wrong Password"}, StatusCode::Unauthorized);
    return QHttpServerResponse({"access_token", webserver->getJwtToken(username)}, StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiAuthRegister(const QHttpServerRequest &request)
{
    auto db       = DataBase::instance();
    auto body     = QJsonDocument::fromJson(request.body()).object();
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
    auto id   = body["user_id"].toInt();
    auto db   = DataBase::instance();
    db->updateUser(id, QList<QPair<QString, QVariant>>() << QPair<QString, QVariant>(body["field"].toString(), body["value"].toVariant()));
    return QHttpServerResponse(StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiUserSetPhoto(const QHttpServerRequest &request)
{
    auto headers  = GlobalCommon::parseHeaders(request.headers());
    auto fileName = headers["filename"];
    auto account  = headers["account"];
    auto id       = headers["user_id"].toInt();
    auto withFile = headers["with_file"] == "true";
    auto db       = DataBase::instance();

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
    if (result.has_value())
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

void GlobalEvent::onWSNewConnection()
{
    auto ws   = GlobalData::instance()->wsServer->nextPendingConnection();
    auto uuid = QUuid::createUuid().toString();
    GlobalData::instance()->wsClients.insert(uuid, QSharedPointer<QWebSocket>(ws));
    QObject::connect(ws, &QWebSocket::textMessageReceived, [this, uuid](const QString &message) { onWSMessageReceived(message, uuid); });
    QObject::connect(ws, &QWebSocket::disconnected, [this, uuid]() { onWSDisconnection(uuid); });
}

void GlobalEvent::onWSDisconnection(const QString &uuid)
{
    GlobalData::instance()->wsClients.remove(uuid);
}

void GlobalEvent::onWSMessageReceived(const QString &message, const QString &uuid)
{
    auto msg_json = QJsonDocument::fromJson(message.toUtf8()).object();
    auto handler  = _wsHandlers[msg_json["type"].toString()];
    std::invoke(handler, this, msg_json["data"], uuid);
}

void GlobalEvent::onWSHandleContainer(QJsonObject &data, const QString &uuid)
{
    auto db = DataBase::instance();
    if data["opt"] == "create":
    {
        auto image_ = db->getImage(data["image_id"].toInt());
        if (!image_.has_value())
            return;
        auto image = image_.value();
        QJsonObject msg {
            {"type", "container"},
            {"data", {
                {"opt", "create"},
                {"user_id", data["user_id"].toInt()},
                {"uuid", uuid},
                {"create_args", {
                    {"name", QString("u%1_c%2").arg(data["account"].toString()).arg(GlobalCommon::generateRandomString(10))},
                    {"image", image["imagename"].toString()},
                    {"port", data["port"]},
                    {"hostname",data["account"].toString()},

                }}
            }}
        };
        auto init_args = image["init_args"].toObject();
        for(auto &key:init_args.keys())
            msg["data"]["create_args"].toObject().insert(key,init_args[key]);
        
    }
}

void GlobalEvent::onNewTcpConnection() 
{
    auto tcpserver = GlobalData::instance()->tcpServer;
    auto socket = tcpserver->nextPendingConnection();
    socket->setProperty("buffer", QByteArray());
    QObject::connect(socket.get(), &QTcpSocket::readyRead, [this, socket]() { onTcpMessageReceived() });
}
//
void GlobalEvent::onTcpMessageReceived()
{
    auto sder = qobject_cast<QTcpSocket *>(sender());
    auto msg  = sder->property("buffer").toByteArray() + sder->readAll();
    if (msg.contains('\x03')) // 为了防止粘包，使用\x03作为分隔符
    {
        auto msgs     = msg.split('\x03');
        auto msg_json = QJsonDocument::fromJson(msgs[0]).object();
        sder->setProperty("buffer", msgs[1]);
        if (msg_json["type"].toString() == "init")
            onTcpHandleInit(msg_json["data"].toObject(), sder);
        else
            std::invoke(_tcpHandlers[msg_json["type"].toString()], this, msg_json["data"].toObject(), sder->property("machine_id").toString());
    }
}

void GlobalEvent::onTcpDisconnection(const QString &machineId)
{
    GlobalData::instance()->tcpClients.remove(machineId);
}

void GlobalEvent::onTcpHandleInit(QJsonObject &data, QTcpSocket *sder)
{
    auto machineId = data["machine_id"].toString();
    sder->setProperty("machine_id", machineId);
    QObject::connect(sder, &QTcpSocket::disconnected, [this, machineId]() { onTcpDisconnection(machineId); });
    GlobalData::instance()->tcpClients.insert(machineId, QSharedPointer<QTcpSocket>(sder));
    auto url    = data["url"].toString();
    auto gpus   = data["gpus"].toObject();
    auto cpu    = data["cpu"].toObject();
    auto memory = data["memory"].toObject();
    auto disk   = data["disk"].toObject();
    auto db     = DataBase::instance();
    if(db->containsMachine(machineId))
        db->updateMachine(machineId, QList<QPair<QString, QVariant>>()
            << QPair<QString, QVariant>("ip", url)
            << QPair<QString, QVariant>("gpu", gpus)
            << QPair<QString, QVariant>("cpu", cpu)
            << QPair<QString, QVariant>("memory", memory)
            << QPair<QString, QVariant>("disk", disk)
            << QPair<QString, QVariant>("online", true));
    else
        db->insertMachine(machineId, url, gpus, cpu, memory, disk, true);
    // TODO: 在任务队列中创建新队列

    auto containers = data["containers"].toArray();
    // TODO: 过滤不符合规范的容器

    for(auto &container:containers){
        auto container_    = container.toObject();
        auto containername = container_["name"].toString();
        auto running       = container_["running"].toBool();
        if(db->containsContainer(containername))
            db->updateContainerRunning(containername, running);
        // 理论上这些容器都会在数据库中存在，因为在创建容器时会在数据库中创建容器
    }
}

void GlobalEvent::onTcpHandleContainer(QJsonObject &data, const QString &machineId)
{
    auto uuid = data["uuid"].toString();
    auto gd  = GlobalData::instance();
    if(gd->wsClients.contains(uuid))
    {
        QJsonObject wsMsg {
            {"type", "container"},
            {"opt",data["opt"]},
            {"status", data["status"]}
        };
        gd->wsClients[uuid]->sendTextMessage(GlobalCommon::objectToString(data));
    }
        
    
}

void GlobalEvent::onTcpHandleGpus(QJsonObject &data, const QString &machineId)
{
    GlobalData::instance()->gpus_cache[machineId] = data;
}

GlobalEvent::GlobalEvent(QObject *parent)
    : QObject{parent}
{
    _wsHandlers["container"] = &GlobalEvent::onWSHandleContainer;
    _tcpHandlers["container"] = &GlobalEvent::onTcpHandleContainer;
    _tcpHandlers["gpus"] = &GlobalEvent::onTcpHandleGpus;
}
