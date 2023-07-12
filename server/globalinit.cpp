#include "globalinit.h"
#include <QWeakPointer>
#include "webserver.h"
QSharedPointer<GlobalInit> GlobalInit::_instance = QSharedPointer<GlobalInit>::create();

QSharedPointer<GlobalInit> GlobalInit::instance()
{
    return _instance;
}

GlobalInit::GlobalInit(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<QWeakPointer<WebServer>>("QWeakPointer<WebServer>");
    qRegisterMetaType<QSharedPointer<WebServer>>("QSharedPointer<WebServer>");

}
