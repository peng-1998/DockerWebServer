#include "globalconfig.h"
using Type = YAML::NodeType;
bool GlobalConfig::_isInit = false;
QSharedPointer<GlobalConfig> GlobalConfig::_instance = nullptr;
GlobalConfig::GlobalConfig(const QString &fileName)
    : QObject{nullptr}, Node{YAML::LoadFile(fileName.toStdString())}
{

}

GlobalConfig::~GlobalConfig()
{

}

void GlobalConfig::init(const QString &fileName)
{
    if(_isInit)
        return;
    _isInit = true;
    _instance = QSharedPointer<GlobalConfig>(new GlobalConfig(fileName));
}


QSharedPointer<GlobalConfig> GlobalConfig::instance()
{
    return GlobalConfig::_instance;
}
