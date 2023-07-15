#include "globalcommon.h"
#include "bcrypt/qtbcrypt.h"
#include <QMetaType>

QMap<QString, QString> GlobalCommon::parseHeaders(const QList<QPair<QByteArray, QByteArray>> &headers)
{
    QMap<QString, QString> result;
    for (auto &header : headers) {
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
        if(value.typeId() == QMetaType::QString)
        {
            result.insert(key, hash.value(key).toString());
        }
        else if(value.typeId() == QMetaType::Int)
        {
            result.insert(key, hash.value(key).toInt());
        }
        else if(value.typeId() == QMetaType::Bool)
        {
            result.insert(key, hash.value(key).toBool());
        }
        else
        {
            result.insert(key, hash.value(key).toJsonObject());
        }
    }
    return result;
}

GlobalCommon::GlobalCommon(QObject *parent)
    : QObject{parent}
{

}
