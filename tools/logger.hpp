#pragma once

#include <QDateTime>
#include <QFile>
#include <QMessageLogContext>
#include <QTextStream>
class Logger
{

private:
    QFile _file;
    QTextStream _stream;
    QStringList _tags;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;

public:
    Logger(const QString &path)
    {
        _file.setFileName(path + "/log.txt");
        _file.open(QIODevice::WriteOnly | QIODevice::Append);
        _stream.setDevice(&_file);
        _tags << "DEBUG"
              << "WARNING"
              << "CRITICAL"
              << "FATAL"
              << "INFO";
    }

    ~Logger()
    {
        _file.close();
    }
    
    void write(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        _stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << " " << _tags[type] << " " << context.file << ":" << context.line << " " << context.function << " " << msg << Qt::endl;
    }
};