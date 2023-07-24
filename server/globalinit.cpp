#include "globalinit.h"
#include <QWeakPointer>
#include "webserver.h"
#include "database.h"

void GlobalInit::registerType()
{
    qRegisterMetaType<QWeakPointer<QWebSocketServer>>("QWeakPointer<QWebSocketServer>");
    qRegisterMetaType<QSharedPointer<QWebSocketServer>>("QSharedPointer<QWebSocketServer>");
}

void GlobalInit::init()
{
    GlobalConfig::init("config.yaml");
    GlobalData::instance();
    GlobalInit::registerType();
    GlobalData::instance()->tcpServer = QSharedPointer<QTcpServer>(new QTcpServer());
    GlobalData::instance()->tcpServer->listen(QHostAddress::Any, (*GlobalConfig::instance())["TCP"]["port"].as<int>());
    connect(GlobalData::instance()->tcpServer.get(), &QTcpServer::newConnection, GlobalEvent::instance().get(), &GlobalEvent::onNewTcpConnection);
    connect(&GlobalData::instance()->heartbeatTimer, &QTimer::timeout, GlobalEvent::instance().get(), &GlobalEvent::onCheckHeartbeat);
    GlobalData::instance()->heartbeatTimer.start(1000);
}

GlobalInit::GlobalInit(QObject *parent)
    : QObject{parent}
{
}
