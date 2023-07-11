#include "globalevent.h"

GlobalEvent *GlobalEvent::_instance = new GlobalEvent;

GlobalEvent *GlobalEvent::instance()
{
    return _instance;
}

GlobalEvent::GlobalEvent(QObject *parent)
    : QObject{parent}
{

}
