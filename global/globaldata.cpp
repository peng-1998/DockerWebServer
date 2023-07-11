#include "globaldata.h"

QSharedPointer<GlobalData> GlobalData::_instance = QSharedPointer<GlobalData>::create();


QSharedPointer<GlobalData> GlobalData::instance()
{
    return _instance;
}

GlobalData::GlobalData()
    : QHash<QString, QVariant>()
{

}
