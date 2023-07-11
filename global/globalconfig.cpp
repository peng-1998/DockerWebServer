#include "globalconfig.h"

bool GlobalConfig::_isInit = false;

GlobalConfig::GlobalConfig(const QString &fileName, Format format = Format::IniFormat)
    : QSettings(fileName, format)
{

}

GlobalConfig::~GlobalConfig()
{

}

void GlobalConfig::init(const QString &fileName, Format format)
{
    if(_isInit)
        return;
    _isInit = true;
    _instance = QSharedPointer<GlobalConfig>::create(fileName,format);
}

QSharedPointer<GlobalConfig> GlobalConfig::instance()
{
    return _instance;
}
