#pragma once

#include <QObject>
#include <QSettings>
#include <QSharedPointer>
#include <yaml-cpp/yaml.h>
#include <QVariant>
#include <QAnyStringView>
using Node = YAML::Node;
class GlobalConfig : public QObject, public Node
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalConfig> instance();
    ~GlobalConfig();
    static void init(const QString &fileName);
signals:

private:
    GlobalConfig(const QString &fileName);
    GlobalConfig(const GlobalConfig &) = delete;
    GlobalConfig &operator=(const GlobalConfig &) = delete;
    GlobalConfig(GlobalConfig &&) = delete;
    static QSharedPointer<GlobalConfig> _instance;
    static bool _isInit;
};
