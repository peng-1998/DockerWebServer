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
    auto username  = body["username"].toString();
    auto db        = DataBase::instance();
    if (!db->containsUser(username))
        return QHttpServerResponse({"message", "User Not Found"}, StatusCode::NotFound);
    auto user = db->getUser(username, QStringList() << "password" << "salt").value();
    if (user["password"].toString() != GlobalCommon::hashPassword(body["password"].toString(), user["salt"].toString()))
        return QHttpServerResponse({"message", "Wrong Password"}, StatusCode::Unauthorized);
    return QHttpServerResponse({"access_token", GlobalCommon::getJwtToken(GlobalData::instance()->jwt, username)}, StatusCode::Ok);
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

QHttpServerResponse GlobalEvent::onApiAdminAllUsers(const QHttpServerRequest &request)
{
    auto users = DataBase::instance()->getUserAll();
    QJsonArray result;
    for (auto &user : users)
        result.append(GlobalCommon::hashToJsonObject(user));
    return QHttpServerResponse(result, StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiAdminAllImages(const QHttpServerRequest &request)
{
    auto images = DataBase::instance()->getImageAll();
    QJsonArray result;
    for (auto &image : images)
        result.append(GlobalCommon::hashToJsonObject(image));
    return QHttpServerResponse(result, StatusCode::Ok);
}

QHttpServerResponse GlobalEvent::onApiAdminAllContainers(const QString &machineId, const QHttpServerRequest &request)
{
    auto containers = DataBase::instance()->getContainerMachine(machineId);
    QJsonArray result;
    for (auto &container : containers)
        result.append(GlobalCommon::hashToJsonObject(container));
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
    auto data     = msg_json["data"].toObject();
    std::invoke(handler, this, data, uuid);
}

void GlobalEvent::onWSHandleContainer(QJsonObject &data, const QString &uuid)
{
    auto db = DataBase::instance();
    if (data["opt"] == "create")
    {
        auto image_ = db->getImage(data["image_id"].toInt());
        if (!image_.has_value())
            return;
        auto image = image_.value();
        QJsonObject msg {
            {"type", "container"},
            {"data", QJsonObject {
                {"opt", "create"},
                {"user_id", data["user_id"].toInt()},
                {"uuid", uuid},
                {"create_args", QJsonObject {
                    {"name", QString("u%1_c%2").arg(data["account"].toString()).arg(GlobalCommon::generateRandomString(10))},
                    {"image", image["imagename"].toString()},
                    {"port", data["port"].toObject()},
                    {"hostname",data["account"].toString()},
                }},
            }},
        };
        auto init_args = QJsonDocument::fromVariant(image["init_args"]).object();
        for(auto &key:init_args.keys())
            msg["data"].toObject()["create_args"].toObject().insert(key,init_args[key]);
        
    }
}

void GlobalEvent::onNewTcpConnection() 
{
    auto tcpserver = GlobalData::instance()->tcpServer;
    auto socket = tcpserver->nextPendingConnection();
    socket->setProperty("buffer", QByteArray());
    QObject::connect(socket, &QTcpSocket::readyRead, [this, socket]() { onTcpMessageReceived(); });
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
        auto data = msg_json["data"].toObject();
        auto machineId = sder->property("machine_id").toString();
        if (msg_json["type"].toString() == "init")
            onTcpHandleInit(data, sder);
        else
            std::invoke(_tcpHandlers[msg_json["type"].toString()], this, data, machineId);
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

    for(auto container:containers){
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
    auto uuid          = data["uuid"].toString();
    auto gd            = GlobalData::instance();
    auto opt           = data["opt"].toString();
    auto db            = DataBase::instance();
    auto containername = data["containername"].toString();
    if(gd->wsClients.contains(uuid))
    {
        QJsonObject wsMsg {
            {"type", "container"},
            {"opt", data["opt"]},
            {"containername", data["containername"]},
            {"status", data["status"]}
        };
        gd->wsClients[uuid]->sendTextMessage(GlobalCommon::objectToString(data));
    }
    if (opt == "create")
        if (auto original_data = data["original_data"].toObject(); data["status"] == "success")
        {
            auto c_sp = containername.split("_");
            auto containername = original_data["containername"].toString();
            auto showname      = c_sp[1].right(c_sp[1].length() - 1);
            auto imagename     = original_data["imagename"].toString();
            auto account       = c_sp[0].right(c_sp[0].length() - 1);
            auto portlist      = original_data["portlist"].toArray();
            db->insertContainer(
                containername,
                showname,
                imagename,
                account,
                machineId,
                portlist,
                false);
        }
        else{
            // TODO: 向日志中写入错误信息
        }
    else if (opt == "start" || opt == "restart")
        db->updateContainerRunning(containername, true);
    else if (opt == "stop")
        db->updateContainerRunning(containername, false);
    else if (opt == "delete")
        db->deleteContainer(containername);
}

void GlobalEvent::onTcpHandleGpus(QJsonObject &data, const QString &machineId)
{
    GlobalData::instance()->gpus_cache[machineId] = data;
}

void GlobalEvent::onTcpHandleImage(QJsonObject &data, const QString &machineId)
{
    if(data.contains("uuid")){
        auto uuid = data["uuid"].toString();
        auto wsClients = GlobalData::instance()->wsClients;
        if(wsClients.contains(uuid))
        {
            QJsonObject wsMsg {
                {"type", "image"},
                {"opt", data["opt"]},
                {"status", data["status"]}
            };
            wsClients[uuid]->sendTextMessage(GlobalCommon::objectToString(wsMsg));
        }
    }
    // TODO: 不知道这里要做什么
    
}

void GlobalEvent::onTcpHandleHeartbeat(QJsonObject &data, const QString &machineId)
{
    GlobalData::instance()->tcpClients[machineId]->setProperty("lastHeartbeat", QDateTime::currentDateTime());
}

void GlobalEvent::onCheckHeartbeat()
{
    auto &clients = GlobalData::instance()->tcpClients;
    if (clients.isEmpty())
        return;
    auto now = QDateTime::currentDateTime();
    auto heartbeatTimeout = (*GlobalConfig::instance())["heartbeatTimeout"].as<int>();
    std::for_each(
        clients.begin(), 
        clients.end(), 
        [now, heartbeatTimeout](auto &client)
        {if (client->property("lastHeartbeat").toDateTime().secsTo(now) > heartbeatTimeout)
            client->disconnectFromHost(); });
}

GlobalEvent::GlobalEvent(QObject *parent)
    : QObject{parent}
{
    _wsHandlers["container"]  = &GlobalEvent::onWSHandleContainer;
    _tcpHandlers["container"] = &GlobalEvent::onTcpHandleContainer;
    _tcpHandlers["gpus"]      = &GlobalEvent::onTcpHandleGpus;
    _tcpHandlers["image"]     = &GlobalEvent::onTcpHandleImage;
    _tcpHandlers["heartbeat"] = &GlobalEvent::onTcpHandleHeartbeat;
}
