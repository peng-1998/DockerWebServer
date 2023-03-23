#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QList>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>
#include <QJsonArray>
class DataBase : public QObject {
    Q_OBJECT
public:
    DataBase(QSettings *settings);
    void addUser(QString name, QString password, QString email = "");
    void loadUsers(QString path);
    void saveUsers(QString path);
    void addLinuxAccount(QString user,QString machine, QString account);
    void delLinuxAccount(QString user, QString machine, QString account);
    void setEmail(QString user, QString email);
    void setPassword(QString user, QString password);
    QString getPassword(QString user);
    QString getEmail(QString user);
    QList<QString> getLinuxAccount(QString user, QString machine);
    QList<QPair<QString, QString>> getLinuxAccount(QString user);
    bool hasUser(QString user);

private:
    QJsonObject _users;
    QSettings *_settings;

signals:
    void dataChanged();
};

#endif // DATABASE_H
