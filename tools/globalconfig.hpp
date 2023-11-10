#pragma once

#include <QAnyStringView>
#include <QObject>
#include <QSettings>
#include <QSharedPointer>
#include <QVariant>
#include <yaml-cpp/yaml.h>

using Node = YAML::Node;

class GlobalConfig : public QObject, public Node
{
    Q_OBJECT
public:
    static GlobalConfig &instance()
    {
        static GlobalConfig _instance;
        return _instance;
    };

    void init(const QString &fileName)
    {
        this->Node::operator=(YAML::LoadFile(fileName.toStdString()));
    };
    ~GlobalConfig() = default;

private:
    GlobalConfig() = default;
    GlobalConfig(const GlobalConfig &) = delete;
    GlobalConfig &operator=(const GlobalConfig &) = delete;
    GlobalConfig(GlobalConfig &&) = delete;
};
