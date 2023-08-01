#include "globaldata.h"

QSharedPointer<GlobalData> GlobalData::_instance = nullptr;

QSharedPointer<GlobalData> GlobalData::instance()
{
    if (_instance.isNull())
        _instance = QSharedPointer<GlobalData>(new GlobalData());
    return _instance;
}