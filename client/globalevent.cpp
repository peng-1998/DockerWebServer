#include "globalevent.h"
#include "globalcommon.h"
#include "../tools/globalconfig.h"
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include "globaldata.h"
QSharedPointer<GlobalEvent> GlobalEvent::_instance = QSharedPointer<GlobalEvent>(new GlobalEvent());

QSharedPointer<GlobalEvent> GlobalEvent::instance()
{
    return _instance;
}

void GlobalEvent::onSendHeartbeat()
{
    auto server = GlobalData::instance()->socket;
    auto config = GlobalConfig::instance();
    QJsonObject msg {{"type", "heartbeat"}, {"data", {}}};
    if (server->isValid())
        server->write(GlobalCommon::formatMessage(msg));
}

void GlobalEvent::onConnected()
{
    auto config = GlobalConfig::instance();
    auto server = GlobalData::instance()->socket;
    auto contaioners = GlobalData::instance()->docker->containers();
    QJsonArray contaionersj;
    for(auto &c : contaioners)
        contaionersj.append(c.toJson());
    QJsonObject msg {
        {"type", "heartbeat"},
        {"data", QJsonObject {
                     {"machineId", QString::fromStdString((*config)["machineId"].as<std::string>())},
                     {"url",QString::fromStdString((*config)["url"].as<std::string>())},
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

}


GlobalEvent::GlobalEvent(QObject *parent) : QObject(parent)
{
    
}