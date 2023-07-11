#pragma once

#include <QObject>
#include <QSettings>
#include <QSharedPointer>
class GlobalConfig :  public QSettings
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalConfig> instance();
    ~GlobalConfig();
    static void init(const QString &fileName, Format format = Format::IniFormat);


signals:

private:
    GlobalConfig(const QString &fileName, Format format = Format::IniFormat);
    GlobalConfig(const GlobalConfig&) = delete;
    GlobalConfig& operator=(const GlobalConfig&) = delete;
    GlobalConfig(GlobalConfig&&) = delete;
    static QSharedPointer<GlobalConfig> _instance;
    static bool _isInit;

};

