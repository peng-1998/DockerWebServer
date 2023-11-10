#include "globalcommon.h"
#include "../tools/bcrypt/qtbcrypt.h"
#include <QJsonDocument>
#include <QMetaType>
#include <QRandomGenerator>
#include <QtEndian>

QMap<QString, QString> GlobalCommon::parseHeaders(const QList<QPair<QByteArray, QByteArray>> &headers)
{
    QMap<QString, QString> result;
    for (auto &header : headers)
        result.insert(QString::fromUtf8(header.first), QString::fromUtf8(header.second));
    return result;
}

QString GlobalCommon::hashPassword(const QString &password, const QString &salt)
{
    return QtBCrypt::hashPassword(password, salt);
}

std::tuple<QString, QString> GlobalCommon::generateSaltAndHash(const QString &password)
{
    auto salt = QtBCrypt::generateSalt();
    return std::make_tuple(salt, QtBCrypt::hashPassword(password, salt));
}

QJsonObject GlobalCommon::hashToJsonObject(const QHash<QString, QVariant> &hash)
{
    QJsonObject result;
    for (auto kv = hash.constKeyValueBegin(); kv != hash.constKeyValueEnd(); kv++)
        result.insert(kv->first, QJsonValue::fromVariant(kv->second));
    return result;
}

QString GlobalCommon::generateRandomString(int length)
{
    static QString characters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    static QRandomGenerator *generator = QRandomGenerator::global();
    QString result(length, QChar(' '));
    std::generate(result.begin(), result.end(), [&]()
                  { return characters.at(generator->bounded(characters.length())); });
    return result;
}

QString GlobalCommon::objectToString(const QJsonObject &object)
{
    return QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

QJsonObject GlobalCommon::stringToObject(const QByteArray &string)
{
    return QJsonDocument::fromJson(string).object();
}

QString GlobalCommon::getJwtToken(QJsonWebToken &jwt, const QString &identity)
{
    jwt.appendClaim("identity", identity);
    // _jwt->appendClaim("iat", QDateTime::currentDateTime().toSecsSinceEpoch());
    // _jwt->appendClaim("exp", QDateTime::currentDateTime().addDays(1).toSecsSinceEpoch());
    auto token = jwt.getToken();
    jwt.removeClaim("identity");
    return token;
}

QList<QPair<QString, QVariant>> GlobalCommon::jsonToKVList(const QJsonObject &json)
{
    QList<QPair<QString, QVariant>> result;
    for (auto kv = json.constBegin(); kv != json.constEnd(); kv++)
        result.append({kv.key(), kv.value().toVariant()});
    return result;
}

QByteArray GlobalCommon::formatMessage(const QJsonObject &json)
{
    auto jsonBytes = QJsonDocument(json).toJson(QJsonDocument::Compact);
    qint32 length = jsonBytes.size();
    static bool isLittleEndian = QSysInfo::ByteOrder == QSysInfo::LittleEndian;
    if (Q_UNLIKELY(isLittleEndian))
        length = qToLittleEndian(length);
    QByteArray lengthBytes = QByteArray::fromRawData(reinterpret_cast<const char *>(&length), sizeof(length));
    return lengthBytes + jsonBytes;
}
