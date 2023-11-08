#include "GE.h"
#include "../tools/dockercontroller.h"
#include "database.h"
#include "globaldata.h"
#include "webserver.h"
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QUuid>
#include <QWebSocket>
#include <QWebSocketServer>

#define globalData GlobalData::instance()
#define globalConfig GlobalConfig::instance()
#define database DataBase::instance()

#define request_ const QHttpServerRequest &request
#define session_ const QString &session

#define toQStr QString::fromStdString
#define hashToObj GlobalCommon::hashToJsonObject
#define jload(x) QJsonDocument::fromJson(x).object()

#define bodyStr(x) body[x].toString()
#define bodyInt(x) body[x].toInt()

#define __STATICsessionCache static auto *sessionCacheStaticPtr = &globalData.session_cache;
#define __STATICdatabase static auto *databaseStaticPtr = &database;
#define __STATICglobalData static auto *globalDataStaticPtr = &globalData;
#define __STATICwsClients static auto *wsClientsStaticPtr = &globalData.wsClients;

#define __GETaccount auto account = sessionCacheStaticPtr->value(session)["account"];

#define __LOADbody auto body = jload(request.body());
#define __LOADheader auto headers = GlobalCommon::parseHeaders(request.headers());

using StatusCode = QHttpServerResponder::StatusCode;
using Response = QHttpServerResponse;
using KVList = QList<QPair<QString, QVariant>>;
using KV = QPair<QString, QVariant>;
using GE = GlobalEvent;

GE &GE::instance()
{
    static GE _instance;
    return _instance;
}

// Response GE::onHttpIndex(request_)
// {
//     return {"Hello World!", StatusCode::Ok};
// }

// Response GE::onHttpWSServer(request_)
// {
//     int wsport = globalConfig["WebSocket"]["port"].as<int>();
//     return Response{"{\"port\":" + QByteArray::number(wsport) + "}", StatusCode::Ok};
// }

// Response GE::onHttpWSClient(request_)
// {
//     int wsport = globalConfig["WebSocket"]["port"].as<int>();
//     return Response{"{\"port\":" + QByteArray::number(wsport) + "}", StatusCode::Ok};
// }

QString getJwtToken(QJsonWebToken &jwt, const QString &identity)
{
    static int duration = globalConfig["JWT"]["duration"].as<int>();

    jwt.appendClaim("identity", identity);                                                     // 设置验证信息：account
    jwt.appendClaim("exp", QDateTime::currentDateTime().addDays(duration).toSecsSinceEpoch()); // 设置过期时间
    auto token = jwt.getToken();                                                               // 生成token
    jwt.removeClaim("identity");                                                               // 清除验证信息
    jwt.removeClaim("exp");
    return token;
}

Response GE::onApiAuthLogin(request_)
{
    static QStringList sel{"password", "salt"};
    __STATICdatabase;

    __LOADbody;
    auto account = bodyStr("account");
    if (!databaseStaticPtr->containsUser(account))
        return {{"message", "User Not Found"}, StatusCode::NotFound};
    auto user = databaseStaticPtr->getUser(account, sel).value();
    if (user["password"].toString() != GlobalCommon::hashPassword(bodyStr("password"), user["salt"].toString()))
        return {{"message", "Wrong Password"}, StatusCode::Unauthorized};
    return {{"access_token", getJwtToken(globalData.jwt, account)}, StatusCode::Ok};
}

Response GE::onApiAuthRegister(request_)
{
    __STATICdatabase;
    static auto defaultPhoto = toQStr(globalConfig["defaultPhoto"].as<std::string>());

    __LOADbody;
    auto account = bodyStr("account");
    auto password = bodyStr("password");
    if (databaseStaticPtr->containsUser(account))
        return {{"message", "User Already Exists"}, StatusCode::Conflict};
    auto [salt, hash] = GlobalCommon::generateSaltAndHash(password);
    databaseStaticPtr->insertUser(account, hash, salt, account, "", "", defaultPhoto);
    return {StatusCode::Ok};
}

Response GE::onApiAuthLogout(request_, session_)
{
    __STATICsessionCache;

    sessionCacheStaticPtr->remove(session);
    return {StatusCode::Ok};
}

Response GE::onApiAuthSession(request_, session_)
{
    __STATICglobalData;
    __STATICsessionCache;
    __GETaccount;

    auto newSession = getJwtToken(globalDataStaticPtr->jwt, account);
    (*sessionCacheStaticPtr)[newSession] = (*sessionCacheStaticPtr)[session];
    globalDataStaticPtr->remove(session);
    return {{"access_token", newSession}, StatusCode::Ok};
}

Response GE::onApiUserSetProfile(request_, session_)
{
    __STATICsessionCache;
    __GETaccount;

    __LOADbody;
    database.updateUser(account, KVList{{body["field"].toString(), body["value"].toVariant()}});
    return {StatusCode::Ok};
}

Response GE::onApiUserSetPhoto(request_, session_)
{
    __STATICsessionCache;
    __STATICglobalData;
    __STATICdatabase;
    static auto customPhotoPath = toQStr(globalConfig["customPhotoPath"].as<std::string>());

    __GETaccount;
    __LOADheader;
    auto fileName = headers["filename"];
    auto withFile = headers["with_file"] == "true";

    if (!withFile)
    {
        databaseStaticPtr->updateUser(account, KVList{{"photo", toQStr(globalConfig["defaultPhotoPath"].as<std::string>()) + "/" + fileName}});
        return {StatusCode::Ok};
    }
    auto savePath = customPhotoPath + "/" + account + ".png";
    databaseStaticPtr->updateUser(account, KVList{{"photo", savePath}});
    QFile file(savePath);
    file.open(QIODevice::WriteOnly);
    file.write(request.body());
    file.close();
    return {StatusCode::Ok};
}

Response GE::onApiUserGetUser(request_, session_)
{
    __STATICdatabase;
    __STATICsessionCache;

    __GETaccount;
    auto result = databaseStaticPtr->getUser(account);
    if (result.has_value())
        return {StatusCode::NotFound};
    return Response(hashToObj(result.value()), StatusCode::Ok);
}

Response GE::onApiMachinesInfo(request_, session_)
{
    __STATICdatabase;

    auto machines = databaseStaticPtr->getMachineAll();
    QJsonArray result;
    for (auto &machine : machines)
        result.append(hashToObj(machine));
    return {result, StatusCode::Ok};
}

Response GE::onApiAdminAllUsers(request_, session_)
{
    __STATICdatabase;

    auto users = databaseStaticPtr->getUserAll();
    QJsonArray result;
    for (auto &user : users)
        result.append(hashToObj(user));
    return {result, StatusCode::Ok};
}

Response GE::onApiAdminAllImages(request_, session_)
{
    __STATICdatabase;

    auto images = databaseStaticPtr->getImageAll();
    QJsonArray result;
    for (auto &image : images)
        result.append(hashToObj(image));
    return {result, StatusCode::Ok};
}

Response GE::onApiAdminAllContainers(const QString &machineId, request_, session_)
{
    __STATICdatabase;

    auto containers = databaseStaticPtr->getContainerMachine(machineId);
    QJsonArray result;
    for (auto &container : containers)
        result.append(hashToObj(container));
    return Response(result, StatusCode::Ok);
}

Response GE::onApiTaskCancel(request_, session_)
{
    __STATICglobalData;

    __LOADbody;
    globalDataStaticPtr->waitQueue.cancelTask(bodyStr("taskId").toLongLong(), bodyStr("machine_id"));
    return {StatusCode::Ok};
}

Response GE::onApiTaskRequest(request_, session_)
{
    __STATICglobalData;

    __LOADbody;
    if (body.contains("gpu_count"))
        globalDataStaticPtr->waitQueue.newTask(
            bodyInt("user_id"),
            bodyStr("machine_id"),
            bodyStr("containername"),
            bodyStr("command"),
            bodyInt("duration"),
            bodyInt("gpu_count"));
    else
    {
        QList<int> gpus;
        for (auto gpu : body["gpus"].toArray())
            gpus.append(gpu.toInt());
        globalDataStaticPtr->waitQueue.newTask(
            bodyInt("user_id"),
            bodyStr("machine_id"),
            bodyStr("containername"),
            bodyStr("command"),
            bodyInt("duration"),
            gpus.size(),
            gpus);
    }
    return {StatusCode::Ok};
}

Response GE::onApiTaskUser(const QString &account, request_)
{
    QJsonArray result;
    // TODO: 查找用户的所有任务 （包括所有机器？）
    return {result, StatusCode::Ok};
}

Response GE::onApiTaskMachine(const QString &machineId, request_)
{
    QJsonArray result;
    // TODO: 查找机器的所有任务
    return {result, StatusCode::Ok};
}

Response GE::onApiAdminAllTasks(request_)
{
    QJsonArray result;
    // TODO: 查找所有任务
    return {result, StatusCode::Ok};
}

void GE::onWSNewConnection()
{
    __STATICwsClients;
    static auto *wsServer = &globalData.wsServer;

    auto ws = wsServer->nextPendingConnection();
    auto uuid = QUuid::createUuid().toString();
    wsClientsStaticPtr->insert(uuid, QSharedPointer<QWebSocket>(ws));
    QObject::connect(ws, &QWebSocket::textMessageReceived, [this, uuid](const QString &message)
                     { onWSMessageReceived(message, uuid); });
    QObject::connect(ws, &QWebSocket::disconnected, [this, uuid]()
                     { onWSDisconnection(uuid); });
}

void GE::onWSDisconnection(const QString &uuid)
{
    __STATICwsClients;

    wsClientsStaticPtr->remove(uuid);
}

void GE::onWSMessageReceived(const QString &message, const QString &uuid)
{
    auto msg_json = jload(message.toUtf8());
    std::invoke(_wsHandlers[msg_json["type"].toString()], msg_json["data"].toObject(), uuid);
}

void GE::onWSHandleContainer(const QJsonObject &data, const QString &uuid)
{
    __STATICdatabase;
    if (data["opt"] == "create")
    {
        auto image_ = databaseStaticPtr->getImage(data["image_id"].toInt());
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
    // TODO 剩余逻辑
}

void GE::onWSHandleImage(const QJsonObject &data, const QString &uuid)
{
    if (data["opt"] == "build")
        DockerController::instance().buildImage(data["dockerfile"].toString(), data["name"].toString());
}

void GE::onNewTcpConnection()
{
    auto socket = globalData.tcpServer->nextPendingConnection();
    socket->setProperty("len", -1);
    QObject::connect(socket, &QTcpSocket::readyRead, [this, socket]()
                     { onTcpMessageReceived(); });
}

void GE::onTcpMessageReceived()
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
    auto msg_json = jload(data);
    if (msg_json["type"].toString() == "init")
        onTcpHandleInit(msg_json["data"].toObject(), sder);
    else
        std::invoke(_tcpHandlers[msg_json["type"].toString()], msg_json["data"].toObject(), sder->property("machine_id").toString());
}

void GE::onTcpDisconnection(const QString &machineId)
{
    globalData.tcpClients.remove(machineId);
}

void GE::onTcpHandleInit(const QJsonObject &data, QTcpSocket *sder)
{
    auto machineId = data["machine_id"].toString();
    sder->setProperty("machine_id", machineId);
    QObject::connect(sder, &QTcpSocket::disconnected, [this, machineId]()
                     { onTcpDisconnection(machineId); });
    globalData.tcpClients.insert(machineId, QSharedPointer<QTcpSocket>(sder));
    auto url = data["url"].toString();
    auto gpus = data["gpus"].toObject();
    auto cpu = data["cpu"].toObject();
    auto memory = data["memory"].toObject();
    auto disk = data["disk"].toObject();
    auto &db = database;
    if (db.containsMachine(machineId))
        db.updateMachine(machineId, KVList{
                                        {"ip", url},
                                        {"gpu", gpus},
                                        {"cpu", cpu},
                                        {"memory", memory},
                                        {"disk", disk},
                                        {"online", true}});
    else
        db.insertMachine(machineId, url, gpus, cpu, memory, disk, true);
    // TODO: 在任务队列中创建新队列

    auto containers = data["containers"].toArray();
    // TODO: 过滤不符合规范的容器

    for (auto container : containers)
    {
        auto container_ = container.toObject();
        auto containername = container_["name"].toString();
        if (db.containsContainer(containername))
            db.updateContainerRunning(containername, container_["running"].toBool());
        // 理论上这些容器都会在数据库中存在，因为在创建容器时会在数据库中创建容器
    }
}

void GE::onTcpHandleContainer(const QJsonObject &data, const QString &machineId)
{
    __STATICdatabase;
    __STATICglobalData;
    __STATICwsClients;

    auto uuid = data["uuid"].toString();
    auto opt = data["opt"].toString();
    auto containername = data["containername"].toString();
    if (wsClientsStaticPtr->contains(uuid))
        (*wsClientsStaticPtr)[uuid]->sendTextMessage(GlobalCommon::objectToString(QJsonObject{
            {"type", "container"},
            {"opt", data["opt"].toString()},
            {"containername", data["containername"].toString()},
            {"status", data["status"].toString()}}));
    if (opt == "create")
        if (auto original_data = data["original_data"].toObject(); data["status"] == "success")
        {
            auto c_sp = containername.split("_");
            databaseStaticPtr->insertContainer(
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
        databaseStaticPtr->updateContainerRunning(containername, true);
    else if (opt == "stop")
        databaseStaticPtr->updateContainerRunning(containername, false);
    else if (opt == "delete")
        databaseStaticPtr->deleteContainer(containername);
}

void GE::onTcpHandleGpus(const QJsonObject &data, const QString &machineId)
{
    static auto *gpus_cache = &globalData.gpus_cache;

    (*gpus_cache)[machineId] = data;
}

void GE::onTcpHandleImage(const QJsonObject &data, const QString &machineId)
{
    if (data.contains("uuid"))
    {
        auto uuid = data["uuid"].toString();
        auto &wsClients = globalData.wsClients;
        if (wsClients.contains(uuid))
            wsClients[uuid]->sendTextMessage(GlobalCommon::objectToString(QJsonObject{
                {"type", "image"},
                {"opt", data["opt"]},
                {"status", data["status"]}}));
    }
    // TODO: 不知道这里要做什么
}

void GE::onTcpHandleHeartbeat(const QJsonObject &data, const QString &machineId)
{
    static auto *tcpClients = &globalData.tcpClients;

    (*tcpClients)[machineId]->setProperty("lastHeartbeat", QDateTime::currentDateTime());
}

void GE::onCheckHeartbeat()
{
    static auto *tcpClients = &globalData.tcpClients;
    static auto heartbeatTimeout = globalConfig["heartbeatTimeout"].as<int>();
    
    if (clients.isEmpty())
        return;
    std::for_each(
        tcpClients->begin(),
        tcpClients->end(),
        [now = QDateTime::currentDateTime(), timeout = heartbeatTimeout](auto &client)
        {if (client->property("lastHeartbeat").toDateTime().secsTo(now) > heartbeatTimeout)
            client->disconnectFromHost(); });
}

void GE::onRunTask(Task task)
{
    __STATICdatabase;
    __STATICglobalData;

    QTimer::singleShot(task.duration * 3600 * 1000, [this, task = std::move(task)]()
                       { onTaskTimeout(task.id, task.machineId); });
    static QStringList sel{"account"};
    auto account = databaseStaticPtr->getUser(task.userId, sel).value()["account"].toString();
    QJsonArray gpuids;
    for (auto gpuId : task.gpuIds)
        gpuids.append(gpuId);
    globalDataStaticPtr->tcpClients[task.machineId]->write(GlobalCommon::formatMessage(QJsonObject{
        {"type", "task"},
        {"data", QJsonObject{
                     {"opt", "run"},
                     {"taskId", QString::number(task.id)},
                     {
                         "account",
                         account,
                     },
                     {"containername", task.containerName},
                     {"gpus", gpuids},
                     {"command", task.command},
                 }}}));
}

void GE::onTaskTimeout(quint64 taskId, const QString &machineId)
{
    __STATICglobalData;

    globalDataStaticPtr->waitQueue->cancelTask(taskId, machineId, true);
    auto client = globalDataStaticPtr->tcpClients[machineId];
    client->write(GlobalCommon::formatMessage(QJsonObject{
        {"type", "task"},
        {"data", QJsonObject{
                     {"opt", "cancel"},
                     {"taskId", QString::number(taskId)},
                 }}}));
}

GE::GE(QObject *parent)
    : QObject{parent}
{
    _wsHandlers["container"] = std::bind(&GE::onWSHandleContainer, this, std::placeholders::_1, std::placeholders::_2);
    _tcpHandlers["container"] = std::bind(&GE::onTcpHandleContainer, this, std::placeholders::_1, std::placeholders::_2);
    _tcpHandlers["gpus"] = std::bind(&GE::onTcpHandleGpus, this, std::placeholders::_1, std::placeholders::_2);
    _tcpHandlers["image"] = std::bind(&GE::onTcpHandleImage, this, std::placeholders::_1, std::placeholders::_2);
    _tcpHandlers["heartbeat"] = std::bind(&GE::onTcpHandleHeartbeat, this, std::placeholders::_1, std::placeholders::_2);
}