#include "globalevent.h"
#include "../tools/globalconfig.h"
#include "../tools/worker.hpp"
#include "globalcommon.h"
#include "globaldata.h"
#include <QDebug>
#include <QFile>
#include <QJsonObject>
QSharedPointer<GlobalEvent> GlobalEvent::_instance = QSharedPointer<GlobalEvent>(new GlobalEvent());

QSharedPointer<GlobalEvent> GlobalEvent::instance()
{
    return _instance;
}

void GlobalEvent::onSendHeartbeat()
{
    auto server = GlobalData::instance()->socket;
    auto config = GlobalConfig::instance();
    QJsonObject msg{{"type", "heartbeat"}, {"data", {}}};
    if (server->isValid())
        server->write(GlobalCommon::formatMessage(msg));
}

void GlobalEvent::onConnected()
{
    auto config = GlobalConfig::instance();
    auto server = GlobalData::instance()->socket;
    server->setProperty("len", -1);
    auto contaioners = GlobalData::instance()->docker->containers();
    QJsonArray contaionersj;
    for (auto &c : contaioners)
        contaionersj.append(c.toJson());
    auto gpus = GlobalData::instance()->gpu->getAllGPUsInfo();
    QJsonObject gpusj;
    for (auto i{0}; i < gpus.size(); i++)
        gpusj.insert(QString::number(i), gpus[i].toJson());
    auto [cpuName, cpuCores] = GlobalCommon::getCPUInfo();
    auto [totalMemory, availableMemory] = GlobalCommon::getMemoryInfo();
    auto [totalDisk, availableDisk] = GlobalCommon::getDiskInfo(QString::fromStdString((*config)["docker"]["path"].as<std::string>()));
    QJsonObject msg{
        {"type", "heartbeat"},
        {"data", QJsonObject{
                     {"gpus", gpusj},
                     {"cpu", QJsonObject{{"name", cpuName}, {"cores", cpuCores}}},
                     {"memory", QJsonObject{{"total", totalMemory}, {"available", availableMemory}}},
                     {"disk", QJsonObject{{"total", totalDisk}, {"available", availableDisk}}},
                     {"machineId", QString::fromStdString((*config)["machineId"].as<std::string>())},
                     {"url", QString::fromStdString((*config)["url"].as<std::string>())},
                     {"contaioners", QJsonValue(contaionersj)}}}};
    server->write(GlobalCommon::formatMessage(msg));
}

void GlobalEvent::onDisconnected()
{
    auto server = GlobalData::instance()->socket;
    server->connectToHost(server->property("host").toString(), server->property("port").toInt());
}

void GlobalEvent::onReadyRead()
{
    auto server = GlobalData::instance()->socket;
    qint32 len = server->property("len").toInt();
    char *buf = new char[sizeof(qint32)];
    if (len == -1)
    {
        if (server->bytesAvailable() < sizeof(qint32))
            return;
        server->read(buf, sizeof(qint32));
        len = *reinterpret_cast<qint32 *>(buf);
        server->setProperty("len", len);
    }
    if (server->bytesAvailable() < len)
        return;
    QByteArray data = server->read(len);
    server->setProperty("len", -1);
    messageHandler(QJsonDocument::fromJson(data).object());
}

void GlobalEvent::messageHandler(const QJsonObject &json)
{
    std::invoke(_messageHandlers[json["type"].toString()], json["data"].toObject());
}

void GlobalEvent::messageHandleImage(const QJsonObject &json)
{
    auto opt = json["opt"].toString();
    QJsonObject msg{
        {"type", "image"},
        {"data", QJsonObject{
                     {"opt", opt},
                     {"image", json["image"].toObject()},
                     {"uuid", json["uuid"].toString()},
                     {"status", "success"}}}};
    std::function<void()> task;
    if (opt == "pull")
        task = [image = json["image"].toString(), &msg]()
        {if (auto res = GlobalData::instance()->docker->pullImage(image);!res.has_value())
                msg["data"].toObject()["status"] = "failed"; };
    else if (opt == "remove")
        task = [image = json["image"].toString(), &msg]()
        { GlobalData::instance()->docker->removeImage(image); };
    QObject::connect(
        Worker::newTask(std::move(task)),
        &Worker::taskFinished,
        this,
        [msg = std::move(msg)]()
        { GlobalData::instance()->socket->write(GlobalCommon::formatMessage(msg)); },
        Qt::QueuedConnection);
}

void GlobalEvent::messageHandleContainer(const QJsonObject &json)
{
    auto opt = json["opt"].toString();
    QJsonObject msg{
        {"type", "container"},
        {"data", QJsonObject{
                     {"opt", opt},
                     {"container", json["container"].toObject()},
                     {"uuid", json["uuid"].toString()},
                     {"status", "success"}}}};
    std::function<void()> task;
    if (opt == "create")
        task = [json]()
        { 
            auto ports_obj = json["ports"].toObject();
            auto ports = QList<QPair<int, int>>();
            for (auto &port : ports_obj.keys())
                ports.append({port.toInt(), ports_obj[port].toInt()});
            GlobalData::instance()->docker->createContainer(json["image"].toString(), json["name"].toString(),json["cmd"].toString(),ports); };
    else if (opt == "start")
        task = [name = json["name"].toString()]()
        { GlobalData::instance()->docker->containerOpt(name, DockerController::ContainerOpt::START); };
    else if (opt == "stop")
        task = [name = json["name"].toString()]()
        { GlobalData::instance()->docker->containerOpt(name, DockerController::ContainerOpt::STOP); };
    else if (opt == "restart")
        task = [name = json["name"].toString()]()
        { GlobalData::instance()->docker->containerOpt(name, DockerController::ContainerOpt::RESTART); };
    else if (opt == "remove")
        task = [name = json["name"].toString()]()
        { GlobalData::instance()->docker->removeContainer(name); };
    else if (opt == "exec")
        task = [json]()
        { GlobalData::instance()->docker->containerExec(json["name"].toString(), json["cmd"].toString()); };
    else if (opt == "commit")
        task = [json]()
        { GlobalData::instance()->docker->containerCommit(json["name"].toString(), json["image"].toString()), GlobalData::instance()->docker->pushImage(json["image"].toString()); };

    QObject::connect(
        Worker::newTask(std::move(task)),
        &Worker::taskFinished,
        this,
        [msg = std::move(msg)]()
        { GlobalData::instance()->socket->write(GlobalCommon::formatMessage(msg)); },
        Qt::QueuedConnection);
}

GlobalEvent::GlobalEvent(QObject *parent) : QObject(parent)
{
    _messageHandlers["image"] = std::bind(&GlobalEvent::messageHandleImage, this, std::placeholders::_1);
    _messageHandlers["container"] = std::bind(&GlobalEvent::messageHandleContainer, this, std::placeholders::_1);
}