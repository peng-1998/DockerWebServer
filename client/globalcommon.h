#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QObject>
#include <QProcess>

class GlobalCommon : public QObject
{
    Q_OBJECT
public:
    static QByteArray formatMessage(const QJsonObject &json);
    static std::tuple<QString, int> getCPUInfo();
    static std::tuple<float, float> getMemoryInfo();
    static std::tuple<float, float> getDiskInfo(const QString &path);
private:
    explicit GlobalCommon(QObject *parent = nullptr) = delete;
    GlobalCommon(const GlobalCommon &) = delete;
    GlobalCommon &operator=(const GlobalCommon &) = delete;
    GlobalCommon(GlobalCommon &&) = delete;
};
