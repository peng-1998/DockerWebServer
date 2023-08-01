#include "globalevent.h"
#include "database.h"
#include "globaldata.h"
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
    return {"Hello World!", StatusCode::Ok};
}

QHttpServerResponse GlobalEvent::onHttpWSServer(const QHttpServerRequest &request)
{
    int wsport = (*GlobalConfig::instance())["WebSocket"]["port"].as<int>();
    return QHttpServerResponse {"{\"port\":" + QByteArray::number(wsport) + "}", StatusCode::Ok};
}

QHttpServerResponse GlobalEvent::onHttpWSClient(const QHttpServerRequest &request)
{
    int wsport = (*GlobalConfig::instance())["WebSocket"]["port"].as<int>();
    return QHttpServerResponse {"{\"port\":" + QByteArray::number(wsport) + "}", StatusCode::Ok};
}

QHttpServerResponse GlobalEvent::onApiAuthLogin(const QHttpServerRequest &request)
{
    static QStringList sel{"password", "salt"};
    auto body     = QJsonDocument::fromJson(request.body()).object();
    auto username = body["username"].toString();
    auto db       = DataBase::instance();
    if (!db->containsUser(username))
        return {{"message", "User Not Found"}, StatusCode::NotFound};
    auto user = db->getUser(username, sel).value();
    if (user["password"].toString() != GlobalCommon::hashPassword(body["password"].toString(), user["salt"].toString()))
        return {{"message", "Wrong Password"}, StatusCode::Unauthorized};
    return {{"access_token", GlobalCommon::getJwtToken(GlobalData::instance()->jwt, username)}, StatusCode::Ok};
}

QHttpServerResponse GlobalEvent::onApiAuthRegister(const QHttpServerRequest &request)
{
    qint8
    auto db       = DataBase::instance();
    auto body     = QJsonDocument::fromJson(request.body()).object();
    auto username = body["username"].toString();
    auto password = body["password"].toString();
    if (db->containsUser(username))
        return {{"message", "User Already Exists"}, StatusCode::Conflict};
    auto [salt, hash] = GlobalCommon::generateSaltAndHash(password);
    db->insertUser(username, hash, salt, username, "", "", QString::fromStdString((*GlobalConfig::instance())["defaultPhoto"].as<std::string>()));
    return {StatusCode::Ok};
}

QHttpServerResponse GlobalEvent::onApiUserSetProfile(const QHttpServerRequest &request)
{
    auto body = QJsonDocument::fromJson(request.body()).object();
    auto id   = body["user_id"].toInt();
    auto db   = DataBase::instance();
    db->updateUser(id, QList<QPair<QString, QVariant>>() << QPair<QString, QVariant>(body["field"].toString(), body["value"].toVariant()));
    return {StatusCode::Ok};
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
    return {StatusCode::Ok};
    auto savePath = QString::fromStdString((*GlobalConfig::instance())["customPhotoPath"].as<std::string>()) + "/" + account + ".png";
    db->updateUser(id, QList<QPair<QString, QVariant>>() << QPair<QString, QVariant>("photo", savePath));
    QFile file(savePath);
    file.open(QIODevice::WriteOnly);
    file.write(request.body());
    file.close();
    return {StatusCode::Ok};
}

QHttpServerResponse GlobalEvent::onApiUserGetUser(const QString &account, const QHttpServerRequest &request)
{
    auto result = DataBase::instance()->getUser(account);
    if (result.has_value())
        return {StatusCode::NotFound};
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

QHttpServerResponse GlobalEvent::onApiTaskCancel(const QHttpServerRequest &request)
{
    auto body = QJsonDocument::fromJson(request.body()).object();
    GlobalData::instance()->waitQueue->cancelTask(body["taskId"].toString().toLongLong(), body["machine_id"].toString());
    return {StatusCode::Ok};
}

QHttpServerResponse GlobalEvent::onApiTaskRequest(const QHttpServerRequest &request)
{
    auto body = QJsonDocument::fromJson(request.body()).object();
    if(body.contains("gpu_count"))
        GlobalData::instance()->waitQueue->newTask(
        body["user_id"].toInt(), 
        body["machine_id"].toString(),
        body["containername"].toString(), 
        body["command"].toString(),
        body["duration"].toInt(),
        body["gpu_count"].toInt());
    else{
        QList<int> gpus;
        for(auto gpu : body["gpus"].toArray())
            gpus.append(gpu.toInt());
        GlobalData::instance()->waitQueue->newTask(
        body["user_id"].toInt(),
        body["machine_id"].toString(),
        body["containername"].toString(),
        body["command"].toString(),
        body["duration"].toInt(),
        gpus.size(),
        gpus);
    }
    return {StatusCode::Ok};
}

QHttpServerResponse GlobalEvent::onApiTaskUser(const QString &account, const QHttpServerRequest &request)
{
    QJsonArray result;
    //TODO: 查找用户的所有任务 （包括所有机器？）
    return {result, StatusCode::Ok};
}

QHttpServerResponse GlobalEvent::onApiTaskMachine(const QString &machineId, const QHttpServerRequest &request)
{
    QJsonArray result;
    //TODO: 查找机器的所有任务 
    return {result, StatusCode::Ok};
}

QHttpServerResponse GlobalEvent::onApiAdminAllTasks(const QHttpServerRequest &request)
{
    QJsonArray result;
    //TODO: 查找所有任务
    return {result, StatusCode::Ok};
}

void GlobalEvent::onWSNewConnection()
{
    auto ws   = GlobalData::instance()->wsServer->nextPendingConnection();
    auto uuid = QUuid::createUuid().toString();
    GlobalData::instance()->wsClients.insert(uuid, QSharedPointer<QWebSocket>(ws));
    QObject::connect(ws, &QWebSocket::textMessageReceived, [this, uuid](const QString &message)
                     { onWSMessageReceived(message, uuid); });
    QObject::connect(ws, &QWebSocket::disconnected, [this, uuid]()
                     { onWSDisconnection(uuid); });
}

void GlobalEvent::onWSDisconnection(const QString &uuid)
{
    GlobalData::instance()->wsClients.remove(uuid);
}

void GlobalEvent::onWSMessageReceived(const QString &message, const QString &uuid)
{
    auto msg_json = QJsonDocument::fromJson(message.toUtf8()).object();
    std::invoke(_wsHandlers[msg_json["type"].toString()], msg_json["data"].toObject(), uuid);
}

void GlobalEvent::onWSHandleContainer(const QJsonObject &data, const QString &uuid)
{
    auto db = DataBase::instance();
    if (data["opt"] == "create")
    {
        auto image_ = db->getImage(data["image_id"].toInt());
        if (!image_.has_value())
            return;
        auto image = image_.value();
        QJsonObject msg{
            {"type", "container"},
            {"data", QJsonObject{
                         {"opt", "create"},
                         {"user_id", data["user_id"].toInt()},
                         {"uuid", uuid},
                         {"create_args", QJsonObject{
                                             {"name", QString("u%1_c%2").arg(data["account"].toString()).arg(GlobalCommon::generateRandomString(10))},
                                             {"image", image["imagename"].toString()},
                                             {"port", data["port"].toObject()},
                                             {"hostname", data["account"].toString()},
                                         }},
                     }},
        };
        auto init_args = QJsonDocument::fromVariant(image["init_args"]).object();
        for (auto &key : init_args.keys())
            msg["data"].toObject()["create_args"].toObject().insert(key, init_args[key]);
    }
}

void GlobalEvent::onNewTcpConnection()
{
    auto socket = GlobalData::instance()->tcpServer->nextPendingConnection();
    socket->setProperty("len", -1);
    QObject::connect(socket, &QTcpSocket::readyRead, [this, socket](){ onTcpMessageReceived(); });
}

void GlobalEvent::onTcpMessageReceived()
{
    auto sder = qobject_cast<QTcpSocket *>(sender());
    qint32 len = sder->property("len").toInt();
    char *buf = new char[sizeof(qint32)];
    if (len == -1)
    {
        if (sder->bytesAvailable() < sizeof(qint32))
            return;
        sder->read(buf, sizeof(qint32));
        len = *reinterpret_cast<qint32 *>(buf);
        sder->setProperty("len", len);
    }
    if (sder->bytesAvailable() < len)
        return;
    QByteArray data = sder->read(len);
    sder->setProperty("len", -1);
    auto msg_json = QJsonDocument::fromJson(data).object();
    if (msg_json["type"].toString() == "init")
        onTcpHandleInit(msg_json["data"].toObject(), sder);
    else
        std::invoke(_tcpHandlers[msg_json["type"].toString()], msg_json["data"].toObject(), sder->property("machine_id").toString());
}

void GlobalEvent::onTcpDisconnection(const QString &machineId)
{
    GlobalData::instance()->tcpClients.remove(machineId);
}

void GlobalEvent::onTcpHandleInit(const QJsonObject &data, QTcpSocket *sder)
{
    auto machineId = data["machine_id"].toString();
    sder->setProperty("machine_id", machineId);
    QObject::connect(sder, &QTcpSocket::disconnected, [this, machineId](){ onTcpDisconnection(machineId); });
    GlobalData::instance()->tcpClients.insert(machineId, QSharedPointer<QTcpSocket>(sder));
    auto url    = data["url"].toString();
    auto gpus   = data["gpus"].toObject();
    auto cpu    = data["cpu"].toObject();
    auto memory = data["memory"].toObject();
    auto disk   = data["disk"].toObject();
    auto db     = DataBase::instance();
    if (db->containsMachine(machineId))
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

    for (auto container : containers)
    {
        auto container_ = container.toObject();
        auto containername = container_["name"].toString();
        if (db->containsContainer(containername))
            db->updateContainerRunning(containername, container_["running"].toBool());
        // 理论上这些容器都会在数据库中存在，因为在创建容器时会在数据库中创建容器
    }
}

void GlobalEvent::onTcpHandleContainer(const QJsonObject &data, const QString &machineId)
{
    auto uuid = data["uuid"].toString();
    auto gd   = GlobalData::instance();
    auto opt  = data["opt"].toString();
    auto db   = DataBase::instance();
    auto containername = data["containername"].toString();
    if (gd->wsClients.contains(uuid))
        gd->wsClients[uuid]->sendTextMessage(GlobalCommon::objectToString(QJsonObject{
            {"type", "container"},
            {"opt", data["opt"].toString()},
            {"containername", data["containername"].toString()},
            {"status", data["status"].toString()}}));
    if (opt == "create")
        if (auto original_data = data["original_data"].toObject(); data["status"] == "success")
        {
            auto c_sp = containername.split("_");
            db->insertContainer(
                original_data["containername"].toString(),
                c_sp[1].right(c_sp[1].length() - 1),
                original_data["imagename"].toString(),
                c_sp[0].right(c_sp[0].length() - 1),
                machineId,
                original_data["portlist"].toArray(),
                false);
        }
        else
        {
            // TODO: 向日志中写入错误信息
        }
    else if (opt == "start" || opt == "restart")
        db->updateContainerRunning(containername, true);
    else if (opt == "stop")
        db->updateContainerRunning(containername, false);
    else if (opt == "delete")
        db->deleteContainer(containername);
}

void GlobalEvent::onTcpHandleGpus(const QJsonObject &data, const QString &machineId)
{
    GlobalData::instance()->gpus_cache[machineId] = data;
}

void GlobalEvent::onTcpHandleImage(const QJsonObject &data, const QString &machineId)
{
    if (data.contains("uuid"))
    {
        auto uuid = data["uuid"].toString(); 
        auto &wsClients = GlobalData::instance()->wsClients;
        if (wsClients.contains(uuid))
            wsClients[uuid]->sendTextMessage(GlobalCommon::objectToString(QJsonObject{
                {"type", "image"},
                {"opt", data["opt"]},
                {"status", data["status"]}}));
    }
    // TODO: 不知道这里要做什么
}

void GlobalEvent::onTcpHandleHeartbeat(const QJsonObject &data, const QString &machineId)
{
    GlobalData::instance()->tcpClients[machineId]->setProperty("lastHeartbeat", QDateTime::currentDateTime());
}

void GlobalEvent::onCheckHeartbeat()
{
    auto &clients = GlobalData::instance()->tcpClients;
    if (clients.isEmpty())
        return;
    static auto heartbeatTimeout = (*GlobalConfig::instance())["heartbeatTimeout"].as<int>();
    std::for_each(
        clients.begin(),
        clients.end(),
        [now = QDateTime::currentDateTime(), timeout = heartbeatTimeout](auto &client)
        {if (client->property("lastHeartbeat").toDateTime().secsTo(now) > heartbeatTimeout)
            client->disconnectFromHost(); });
}

void GlobalEvent::onRunTask(Task task)
{
    QTimer::singleShot(task.duration * 3600 * 1000, [this, task = std::move(task)]()
                       { onTaskTimeout(task.id, task.machineId); });
    static QStringList sel{"account"};
    auto account = DataBase::instance()->getUser(task.userId, sel).value()["account"].toString();
    GlobalData::instance()->tcpClients[task.machineId]->write(GlobalCommon::formatMessage(QJsonObject{
        {"type", "task"},
        {"data", QJsonObject{
                     {"opt", "run"},
                     {"taskId", task.id},
                     {"account",account,},
                     {"containername", task.containername},
                     {"gpus", QJsonArray::fromVariantList(task.gpus)},
                     {"command", task.command},
                 }}}));
}

void GlobalEvent::onTaskTimeout(quint64 taskId, const QString &machineId)
{
    auto gd = GlobalData::instance();
    gd->waitQueue->cancelTask(taskId, machineId,true);
    auto client = gd->tcpClients[machineId];
    client->write(GlobalCommon::formatMessage(QJsonObject{
        {"type", "task"},
        {"data", QJsonObject{
                     {"opt", "cancel"},
                     {"taskId", taskId},
                 }}}));
}

GlobalEvent::GlobalEvent(QObject *parent)
    : QObject{parent}
{
    _wsHandlers["container"]  = std::bind(&GlobalEvent::onWSHandleContainer, this, std::placeholders::_1, std::placeholders::_2);
    _tcpHandlers["container"] = std::bind(&GlobalEvent::onTcpHandleContainer, this, std::placeholders::_1, std::placeholders::_2);
    _tcpHandlers["gpus"]      = std::bind(&GlobalEvent::onTcpHandleGpus, this, std::placeholders::_1, std::placeholders::_2);
    _tcpHandlers["image"]     = std::bind(&GlobalEvent::onTcpHandleImage, this, std::placeholders::_1, std::placeholders::_2);
    _tcpHandlers["heartbeat"] = std::bind(&GlobalEvent::onTcpHandleHeartbeat, this, std::placeholders::_1, std::placeholders::_2);
}