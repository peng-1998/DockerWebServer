#include "globalcommon.h"
#include <QJsonDocument>

QByteArray GlobalCommon::formatMessage(QJsonObject &json)
{
    auto jsonBytes = QJsonDocument(json).toJson(QJsonDocument::Compact);
    qint32 length = jsonBytes.size();
    QByteArray lengthBytes = QByteArray::fromRawData(reinterpret_cast<const char *>(&length), sizeof(length));
    return lengthBytes + jsonBytes;
}