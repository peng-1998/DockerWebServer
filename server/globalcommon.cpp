#include "globalcommon.h"

GlobalCommon * GlobalCommon::_instance = new GlobalCommon;

GlobalCommon * GlobalCommon::instance()
{
    return _instance;
}

GlobalCommon::GlobalCommon(QObject *parent)
    : QObject{parent}
{

}
