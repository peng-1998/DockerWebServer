#include "globalevent.h"
#include "../tools/globalconfig.h"
#include "globalcommon.h"
#include "globaldata.h"
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QtConcurrent>


GlobalEvent& GlobalEvent::instance()
{
    static GlobalEvent _instance;
    return _instance;
}

void GlobalEvent::onSendHeartbeat()
{
    auto server = GlobalData::instance().socket;
    QJsonObject msg{{"type", "heartbeat"}, {"data", {}}};
    if (server->isValid())
        server->write(GlobalCommon::formatMessage(msg));
}

void GlobalEvent::onConnected()
{
    auto server = GlobalData::instance().socket;
    server->setProperty("len", -1);
    auto contaioners = DockerController::instance().containers();
    QJsonArray contaionersj;
    for (auto &c : contaioners)
        contaionersj.append(c.toJson());
    auto gpus = GlobalData::instance().gpu->getAllGPUsInfo();
    QJsonObject gpusj;
    for (auto i{0}; i < gpus.size(); i++)
        gpusj.insert(QString::number(i), gpus[i].toJson());
    auto [cpuName, cpuCores] = GlobalCommon::getCPUInfo();
    auto [totalMemory, availableMemory] = GlobalCommon::getMemoryInfo();
    auto [totalDisk, availableDisk] = GlobalCommon::getDiskInfo(QString::fromStdString(GlobalConfig::instance()["docker"]["path"].as<std::string>()));
    QJsonObject msg{
        {"type", "heartbeat"},
        {"data", QJsonObject{
                     {"gpus", gpusj},
                     {"cpu", QJsonObject{{"name", cpuName}, {"cores", cpuCores}}},
                     {"memory", QJsonObject{{"total", totalMemory}, {"available", availableMemory}}},
                     {"disk", QJsonObject{{"total", totalDisk}, {"available", availableDisk}}},
                     {"machineId", QString::fromStdString(GlobalConfig::instance()["machineId"].as<std::string>())},
                     {"url", QString::fromStdString(GlobalConfig::instance()["url"].as<std::string>())},
                     {"contaioners", QJsonValue(contaionersj)}}}};
    server->write(GlobalCommon::formatMessage(msg));
}

void GlobalEvent::onDisconnected()
{
    auto server = GlobalData::instance().socket;
    server->connectToHost(server->property("host").toString(), server->property("port").toInt());
}

void GlobalEvent::onReadyRead()
{
    auto server = GlobalData::instance().socket;
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
    QJsonObject * msg = new QJsonObject{
        {"type", "image"},
        {"data", QJsonObject{
                     {"opt", opt},
                     {"image", json["image"].toObject()},
                     {"uuid", json["uuid"].toString()},
                     {"status", "success"}}}};
    std::function<void()> task;
    if (opt == "pull")
        task = [image = json["image"].toString(), msg]()
        {if (auto res = DockerController::instance().pullImage(image);!res.has_value())
                (*msg)["data"].toObject()["status"] = "failed"; };
    else if (opt == "remove")
        task = [image = json["image"].toString(), &msg]()
        { DockerController::instance().removeImage(image); };
    QtConcurrent::run(std::move(task)).then(this, [msg]()
                                            { GlobalData::instance().socket->write(GlobalCommon::formatMessage(*msg)); delete msg; });
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
            DockerController::instance().createContainer(json["image"].toString(), json["name"].toString(),json["cmd"].toString(),ports); };
    else if (opt == "start")
        task = [name = json["name"].toString()]()
        { DockerController::instance().containerOpt(name, DockerController::ContainerOpt::START); };
    else if (opt == "stop")
        task = [name = json["name"].toString()]()
        { DockerController::instance().containerOpt(name, DockerController::ContainerOpt::STOP); };
    else if (opt == "restart")
        task = [name = json["name"].toString()]()
        { DockerController::instance().containerOpt(name, DockerController::ContainerOpt::RESTART); };
    else if (opt == "remove")
        task = [name = json["name"].toString()]()
        { DockerController::instance().removeContainer(name); };
    else if (opt == "exec")
        task = [json]()
        { DockerController::instance().containerExec(json["name"].toString(), json["cmd"].toString()); };
    else if (opt == "commit")
        task = [json]()
        { DockerController::instance().containerCommit(json["name"].toString(), json["image"].toString()), DockerController::instance().pushImage(json["image"].toString()); };
    QtConcurrent::run(std::move(task)).then(this, [msg]()
    { GlobalData::instance().socket->write(GlobalCommon::formatMessage(msg)); });
}

GlobalEvent::GlobalEvent(QObject *parent) : QObject(parent)
{
    _messageHandlers["image"] = std::bind(&GlobalEvent::messageHandleImage, this, std::placeholders::_1);
    _messageHandlers["container"] = std::bind(&GlobalEvent::messageHandleContainer, this, std::placeholders::_1);
}