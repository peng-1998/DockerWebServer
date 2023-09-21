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
    static GlobalConfig &instance();
    ~GlobalConfig() = default;
    void init(const QString &fileName);

private:
    GlobalConfig() = default;
    GlobalConfig(const QString &fileName);
    GlobalConfig(const GlobalConfig &) = delete;
    GlobalConfig &operator=(const GlobalConfig &) = delete;
    GlobalConfig(GlobalConfig &&) = delete;
};
