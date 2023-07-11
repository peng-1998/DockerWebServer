#pragma once

#include <QObject>
#include <QSharedPointer>
class GlobalInit : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalInit> instance();

signals:

private:
    explicit GlobalInit(QObject *parent = nullptr);
    GlobalInit(const GlobalInit&) = delete;
    GlobalInit& operator=(const GlobalInit&) = delete;
    GlobalInit(GlobalInit&&) = delete;
    static QSharedPointer<GlobalInit> _instance;

};

