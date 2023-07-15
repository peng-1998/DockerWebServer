#include "globaldata.h"
#include <QString>
#include <QVariant>

QSharedPointer<GlobalData> GlobalData::_instance = QSharedPointer<GlobalData>(new GlobalData());

QSharedPointer<GlobalData> GlobalData::instance()
{
    return _instance;
}

GlobalData::~GlobalData()
{

}

GlobalData::GlobalData()
    : QObject(), QHash<QString, QVariant>()
{

}
