#include "globaldata.h"

GlobalData* GlobalData::_instance = new GlobalData;

GlobalData * GlobalData::instance()
{
    return _instance;
}

GlobalData::GlobalData()
    : QHash<QString, QVariant>()
{

}
