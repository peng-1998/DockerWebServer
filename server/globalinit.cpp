#include "globalinit.h"

QSharedPointer<GlobalInit> GlobalInit::_instance = QSharedPointer<GlobalInit>::create();

QSharedPointer<GlobalInit> GlobalInit::instance()
{
    return _instance;
}

GlobalInit::GlobalInit(QObject *parent)
    : QObject{parent}
{

}
