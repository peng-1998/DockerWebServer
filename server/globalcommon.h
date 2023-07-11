#pragma once

#include <QObject>

class GlobalCommon : public QObject
{
    Q_OBJECT
public:
    static GlobalCommon * instance();

private:
    explicit GlobalCommon(QObject *parent = nullptr);
    GlobalCommon(const GlobalCommon&) = delete;
    GlobalCommon& operator=(const GlobalCommon&) = delete;
    GlobalCommon(GlobalCommon&&) = delete;
    static GlobalCommon * _instance;

};

