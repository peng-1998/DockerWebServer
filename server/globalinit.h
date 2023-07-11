#pragma once

#include <QObject>

class GlobalInit : public QObject
{
    Q_OBJECT
public:
    static GlobalInit & instance();

signals:

private:
    explicit GlobalInit(QObject *parent = nullptr);
    GlobalInit(const GlobalInit&) = delete;
    GlobalInit& operator=(const GlobalInit&) = delete;
    GlobalInit(GlobalInit&&) = delete;
    static GlobalInit _instance;

};

