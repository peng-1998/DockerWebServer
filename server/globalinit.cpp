#include "globalinit.h"

GlobalInit GlobalInit::_instance;

GlobalInit & GlobalInit::instance()
{
    return _instance;
}

GlobalInit::GlobalInit(QObject *parent)
    : QObject{parent}
{

}
