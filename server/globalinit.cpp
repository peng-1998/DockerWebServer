#include "globalinit.h"
#include <QWeakPointer>
#include "webserver.h"
#include "database.h"

void GlobalInit::registerType()
{
    qRegisterMetaType<QWeakPointer<WebServer>>("QWeakPointer<WebServer>");
    qRegisterMetaType<QSharedPointer<WebServer>>("QSharedPointer<WebServer>");
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
}

GlobalInit::GlobalInit(QObject *parent)
    : QObject{parent}
{
}
