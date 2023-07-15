#include "globalinit.h"
#include <QWeakPointer>
#include "webserver.h"
#include "database.h"

void GlobalInit::registerType()
{
    qRegisterMetaType<QWeakPointer<WebServer>>("QWeakPointer<WebServer>");
    qRegisterMetaType<QSharedPointer<WebServer>>("QSharedPointer<WebServer>");
}

void GlobalInit::init()
{
    GlobalConfig::init("config.yaml");
    GlobalData::instance();
    GlobalInit::registerType();
}

GlobalInit::GlobalInit(QObject *parent)
    : QObject{parent}
{
}
