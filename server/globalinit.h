#pragma once

#include <QObject>
#include <QSharedPointer>
#include "globaldata.h"
class GlobalInit : public QObject
{
    Q_OBJECT
public:
    static void init();
signals:

private:
    explicit GlobalInit(QObject *parent = nullptr);
    GlobalInit(const GlobalInit&) = delete;
    GlobalInit& operator=(const GlobalInit&) = delete;
    GlobalInit(GlobalInit&&) = delete;
    static QSharedPointer<GlobalInit> _instance;
    static void registerType();
};
