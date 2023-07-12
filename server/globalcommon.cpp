#include "globalcommon.h"
#include "bcrypt/qtbcrypt.h"
QSharedPointer<GlobalCommon> GlobalCommon::_instance = QSharedPointer<GlobalCommon>::create();

QSharedPointer<GlobalCommon> GlobalCommon::instance()
{
    return _instance;
}

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
    auto hash = QtBCrypt::hashPassword(password, salt);
    return std::make_tuple(salt, hash);
}

GlobalCommon::GlobalCommon(QObject *parent)
    : QObject{parent}
{

}
