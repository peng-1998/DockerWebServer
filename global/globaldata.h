#pragma once

#include <QObject>
#include <QHash>
#include <QSharedPointer>
class GlobalData : QHash<QString,QVariant>
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalData> instance();
    ~GlobalData();

private:
    GlobalData();
    GlobalData(const GlobalData&) = delete;
    GlobalData& operator=(const GlobalData&) = delete;
    GlobalData(GlobalData&&) = delete;
    static QSharedPointer<GlobalData> _instance;


};

