#pragma once

#include <QObject>
#include <QHash>

class GlobalData : QHash<QString,QVariant>
{
    Q_OBJECT
public:
    static GlobalData * instance();
    ~GlobalData();

private:
    GlobalData();
    GlobalData(const GlobalData&) = delete;
    GlobalData& operator=(const GlobalData&) = delete;
    GlobalData(GlobalData&&) = delete;
    static GlobalData * _instance;


};

