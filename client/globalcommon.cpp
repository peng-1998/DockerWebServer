#include "globalcommon.h"
#include <QJsonDocument>
#include <QStorageInfo>
#include <QSysInfo>
#include <QtEndian>

QByteArray GlobalCommon::formatMessage(const QJsonObject &json)
{
    auto jsonBytes = QJsonDocument(json).toJson(QJsonDocument::Compact);
    qint32 length = jsonBytes.size();
    if (std::endian::native == std::endian::big)
        length = qToLittleEndian(length);
    QByteArray lengthBytes = QByteArray::fromRawData(reinterpret_cast<const char *>(&length), sizeof(length));
    return lengthBytes + jsonBytes;
}

std::tuple<QString, int> GlobalCommon::getCPUInfo()
{
    // 检查是否为 linux 或者 freebsd 系统
    if ((QSysInfo::kernelType() != "linux" and QSysInfo::kernelType() != "freebsd") or !QFile::exists("/proc/cpuinfo"))
        return std::make_tuple("Unknown", 0);
    QProcess process;
    process.start("cat /proc/cpuinfo | grep 'model name' | uniq");
    process.waitForFinished();
    QString result = process.readAllStandardOutput();
    result = result.split(":")[1].trimmed();
    process.start("cat /proc/cpuinfo | grep 'cpu cores' | uniq");
    process.waitForFinished();
    int cores = QString(process.readAllStandardOutput()).split(":")[1].trimmed().toInt();
    return std::make_tuple(result, cores);
}

std::tuple<float, float> GlobalCommon::getMemoryInfo()
{
    // 检查是否为 linux 或者 freebsd 系统
    if ((QSysInfo::kernelType() != "linux" and QSysInfo::kernelType() != "freebsd") or !QFile::exists("/proc/meminfo"))
        return std::make_tuple(0, 0);
    QProcess process;
    process.start("cat /proc/meminfo | grep 'MemTotal'");
    process.waitForFinished();
    float total = QString(process.readAllStandardOutput()).split(":")[1].trimmed().split(" ")[0].toInt() / 1024 / 1024;
    process.start("cat /proc/meminfo | grep 'MemAvailable'");
    process.waitForFinished();
    float available = QString(process.readAllStandardOutput()).split(":")[1].trimmed().split(" ")[0].toInt() / 1024 / 1024;
    return std::make_tuple(total, available);
}

std::tuple<float, float> GlobalCommon::getDiskInfo(const QString &path)
{
    // 检查是否为 linux 或者 freebsd 系统
    QStorageInfo storage(path);
    if (storage.isValid())
        return std::make_tuple(storage.bytesTotal() / 1024 / 1024 / 1024, storage.bytesAvailable() / 1024 / 1024 / 1024);
    return std::make_tuple(0, 0);
}