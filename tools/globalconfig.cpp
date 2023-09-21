#include "globalconfig.h"

GlobalConfig::GlobalConfig(const QString &fileName)
    : QObject{nullptr}, Node{YAML::LoadFile(fileName.toStdString())}
{
}

void GlobalConfig::init(const QString &fileName)
{
    this->Node::operator=(YAML::LoadFile(fileName.toStdString()));
}

GlobalConfig& GlobalConfig::instance()
{
    static GlobalConfig _instance;
    return _instance;
}
