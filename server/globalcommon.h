#pragma once

#include "../tools/jsonwebtoken/src/qjsonwebtoken.h"
#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QSharedPointer>
#include <QThread>

class GlobalCommon : public QObject
{
    Q_OBJECT
public:
    static QMap<QString, QString> parseHeaders(const QList<QPair<QByteArray, QByteArray>> &headers);
    static QString hashPassword(const QString &password, const QString &salt);
    static std::tuple<QString, QString> generateSaltAndHash(const QString &password);
    static QJsonObject hashToJsonObject(const QHash<QString, QVariant> &hash);
    static QString generateRandomString(int length = 32);
    static QString objectToString(const QJsonObject &object);
    static QJsonObject stringToObject(const QByteArray &string);
    static QString getJwtToken(QJsonWebToken &, const QString &);
    static QByteArray formatMessage(const QJsonObject &json);

private:
    explicit GlobalCommon(QObject *parent = nullptr) = delete;
    GlobalCommon(const GlobalCommon &) = delete;
    GlobalCommon &operator=(const GlobalCommon &) = delete;
    GlobalCommon(GlobalCommon &&) = delete;
};
