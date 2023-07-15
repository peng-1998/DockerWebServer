#pragma once

#include <QHash>
#include <QObject>
#include <QSharedPointer>
class GlobalData :public QObject, public QHash<QString, QVariant>
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalData> instance();
    ~GlobalData();

private:
    GlobalData();
    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
    static QSharedPointer<GlobalData> _instance;
};
