#include "globalconfig.h"

bool GlobalConfig::_isInit = false;

QSharedPointer<GlobalConfig> GlobalConfig::_instance = nullptr;

GlobalConfig::GlobalConfig(const QString &fileName)
    : QObject{nullptr}, Node{YAML::LoadFile(fileName.toStdString())}
{
}

void GlobalConfig::init(const QString &fileName)
{
    if (_isInit)
        return;
    _isInit = true;
    _instance = QSharedPointer<GlobalConfig>(new GlobalConfig(fileName));
}

QSharedPointer<GlobalConfig> GlobalConfig::instance()
{
    if (!_isInit)
        throw std::runtime_error("GlobalConfig is not init, please call GlobalConfig::init() first");
    return GlobalConfig::_instance;
}
