#include "globalcommon.h"

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

GlobalCommon::GlobalCommon(QObject *parent)
    : QObject{parent}
{

}
