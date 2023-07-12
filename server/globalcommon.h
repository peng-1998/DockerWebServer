#pragma once

#include <QObject>
#include <QMap>
#include <QPair>
#include <QByteArray>
#include <QSharedPointer>
class GlobalCommon : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalCommon> instance();
    static QMap<QString, QString> parseHeaders(const QList<QPair<QByteArray, QByteArray>> &headers);
    static QString hashPassword(const QString &password, const QString &salt);
    static std::tuple<QString, QString> generateSaltAndHash(const QString &password);
private:
    explicit GlobalCommon(QObject *parent = nullptr);
    GlobalCommon(const GlobalCommon&) = delete;
    GlobalCommon& operator=(const GlobalCommon&) = delete;
    GlobalCommon(GlobalCommon&&) = delete;
    static QSharedPointer<GlobalCommon> _instance;

};

