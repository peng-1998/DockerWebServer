#include "globalevent.h"
#include "../tools/Worker.h"
#include "../tools/globalconfig.h"
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
    QJsonObject msg{
        {"type", "heartbeat"},
        {"data", QJsonObject{
                     {"machineId", QString::fromStdString((*config)["machineId"].as<std::string>())},
                     {"url", QString::fromStdString((*config)["url"].as<std::string>())},
                     {"contaioners", QJsonValue(contaionersj)}}}};
    //  {"gpus", GlobalCommon::getGpusInfo()},
    //  {"cpu", GlobalCommon::getCpuInfo()},
    //  {"memory", GlobalCommon::getMemoryInfo()},
    //  {"disk", GlobalCommon::getDiskInfo()},
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
            return server->read(buf, sizeof(qint32));
        len = *reinterpret_cast<qint32 *>(buf);
        server->setProperty("len", len);
    }
    if (server->bytesAvailable() < len)
        return;
    QByteArray data = server->read(len);
    server->setProperty("len", -1);
    messageHandler(QJsonDocument::fromJson(data).object());
}

void GlobalEvent::messageHandler(QJsonObject json)
{
    std::invoke(_messageHandlers[json["type"].toString()], this, json["data"].toObject());
}

void GlobalEvent::messageHandleImage(QJsonObject json)
{
    auto opt = json["opt"].toString();
    QJsonObject msg{
        {"type", "image"},
        {"data", QJsonObject{
                     {"opt", opt},
                     {"image", json["image"].toObject()},
                     {"uuid", json["uuid"].toString()},
                     {"status", "success"}}}};

    if (opt == "pull")
        auto task = [&json, &msg]()
        {if (auto image = GlobalData::instance()->docker->pullImage(json["image"].toString());!image.has_value())
                msg["data"].toObject()["status"] = "failed"; };
    else if (opt == "remove")
        auto task = [&json, &msg]()
        { GlobalData::instance()->docker->removeImage(json["image"].toString()); };
    auto worker = new Worker(std::move(task));
    connect(
        worker,
        &Worker::taskFinished,
        this,
        [&msg]()
        { GlobalData::instance()->socket->write(GlobalCommon::formatMessage(msg)); },
        Qt::QueuedConnection);
}

void GlobalEvent::messageHandleContainer(const QJsonObject json)
{
    auto opt = json["opt"].toString();
    QJsonObject msg{
        {"type", "container"},
        {"data", QJsonObject{
                     {"opt", opt},
                     {"container", json["container"].toObject()},
                     {"uuid", json["uuid"].toString()},
                     {"status", "success"}}}};
    if (opt == "create")
        auto task = [&json, &msg]()
        { 
            auto ports_obj = json["ports"].toObject();
            auto ports = QList<QPair<int, int>>();
            for (auto &port : ports_obj.keys())
                ports.append({port.toInt(), ports_obj[port].toInt()});
            GlobalData::instance()->docker->createContainer(json["image"].toObject(), json["name"].toString(),json["cmd"],ports); };
    else if (opt == "start")
        auto task = [&json, &msg]()
        { GlobalData::instance()->docker->containerOpt(json["name"].toString(), DockerController::ContainerOpt::START); };
    else if (opt == "stop")
        auto task = [&json, &msg]()
        { GlobalData::instance()->docker->containerOpt(json["name"].toString(), DockerController::ContainerOpt::STOP); };
    else if (opt == "remove")
        auto task = [&json, &msg]()
        { GlobalData::instance()->docker->removeContainer(json["container"].toString()); };
    else if (opt == "restart")
        auto task = [&json, &msg]()
        { GlobalData::instance()->docker->containerOpt(json["name"].toString(), DockerController::ContainerOpt::RESTART); };
    else if (opt == "exec")
        auto task = [&json, &msg]()
        { GlobalData::instance()->docker->containerExec(json["name"].toString(), json["cmd"].toString()); };
    else if (opt == "commit")
        auto task = [&json, &msg]()
        {
            GlobalData::instance()->docker->containerCommit(json["name"].toString(), json["image"].toString());
            GlobalData::instance()->docker->pushImage(json["image"].toString());
        };
    
    auto worker = new Worker(std::move(task));
    connect(
        worker,
        &Worker::taskFinished,
        this,
        [&msg]()
        { GlobalData::instance()->socket->write(GlobalCommon::formatMessage(msg)); },
        Qt::QueuedConnection);
}

GlobalEvent::GlobalEvent(QObject *parent) : QObject(parent)
{
    _messageHandlers["image"] = &GlobalEvent::messageHandleImage;
    _messageHandlers["container"] = &GlobalEvent::messageHandleContainer;
}