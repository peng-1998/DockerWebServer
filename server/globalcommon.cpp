#include "globalcommon.h"
#include "../tools/bcrypt/qtbcrypt.h"
#include <QJsonDocument>
#include <QMetaType>
#include <QRandomGenerator>

QMap<QString, QString> GlobalCommon::parseHeaders(const QList<QPair<QByteArray, QByteArray>> &headers)
{
    QMap<QString, QString> result;
    for (auto &header : headers)
    {
        result.insert(QString::fromUtf8(header.first), QString::fromUtf8(header.second));
    }
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
    for (auto &key : hash.keys())
    {
        auto value = hash.value(key);
        if (value.typeId() == QMetaType::QString)
            result.insert(key, hash.value(key).toString());
        else if (value.typeId() == QMetaType::Int)
            result.insert(key, hash.value(key).toInt());
        else if (value.typeId() == QMetaType::Bool)
            result.insert(key, hash.value(key).toBool());
        else
            result.insert(key, hash.value(key).toJsonObject());
    }
    return result;
}

QString GlobalCommon::generateRandomString(int length)
{
    QString result;
    const QString characters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

    // 使用 QRandomGenerator 生成随机数
    QRandomGenerator generator = QRandomGenerator::securelySeeded();

    // 生成指定长度的随机字符串
    for (int i = 0; i < length; ++i)
    {
        int index = generator.bounded(characters.length());
        result.append(characters.at(index));
    }

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

QString GlobalCommon::getJwtToken(QSharedPointer<QJsonWebToken> jwt, const QString &identity)
{
    jwt->appendClaim("identity", identity);
    // _jwt->appendClaim("iat", QDateTime::currentDateTime().toSecsSinceEpoch());
    // _jwt->appendClaim("exp", QDateTime::currentDateTime().addDays(1).toSecsSinceEpoch());
    return jwt->getToken();
}

QByteArray GlobalCommon::formatMessage(QJsonObject &json)
{
    auto jsonBytes = QJsonDocument(json).toJson(QJsonDocument::Compact);
    qint32 length = jsonBytes.size();
    QByteArray lengthBytes = QByteArray::fromRawData(reinterpret_cast<const char *>(&length), sizeof(length));
    return lengthBytes + jsonBytes;
}

