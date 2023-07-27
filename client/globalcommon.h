#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QObject>


class GlobalCommon : public QObject
{
    Q_OBJECT
public:
    static QByteArray formatMessage(QJsonObject &json);
private:
    explicit GlobalCommon(QObject *parent = nullptr) = delete;
    GlobalCommon(const GlobalCommon &) = delete;
    GlobalCommon &operator=(const GlobalCommon &) = delete;
    GlobalCommon(GlobalCommon &&) = delete;
};
