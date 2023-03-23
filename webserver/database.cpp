#include "database.h"
#include <QFile>
DataBase::DataBase(QSettings *settings) {
    _settings = settings;
    this->loadUsers(_settings->value("database/users").toString());
    connect(this, &DataBase::dataChanged, this, [=](){
        this->saveUsers(_settings->value("database/users").toString());
    });
}

void DataBase::addUser(QString name, QString password, QString email) {
    QJsonObject user;
    user.insert("password", password);
    user.insert("email", email);
    user.insert("linuxAccount", QJsonArray());
    this->_users.insert(name, user);
}

void DataBase::loadUsers(QString path) {
    QFile file{path};
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    auto data = file.readAll();
    auto jsondata = QJsonDocument::fromJson(data);
    if (jsondata.isNull()) {
        return;
    }
    this->_users = jsondata.object();
}

void DataBase::saveUsers(QString path) {
    QFile file{path};
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    auto jsondata = QJsonDocument(this->_users);
    file.write(jsondata.toJson(QJsonDocument::Compact));
}

void DataBase::addLinuxAccount(QString user, QString machine, QString account) {
    if(!this->_users.contains(user)) {
        return;
    }
    auto userobj = this->_users.value(user).toObject();
    auto linuxAccount = userobj.value("linuxAccount").toArray();
    linuxAccount.append(QJsonObject{{"machine", machine}, {"account", account}});
    userobj.insert("linuxAccount", linuxAccount);
    this->_users.insert(user, userobj);
    emit this->dataChanged();
}

void DataBase::delLinuxAccount(QString user, QString machine, QString account) {
    if(!this->_users.contains(user)) {
        return;
    }
    auto userobj = this->_users.value(user).toObject();
    auto linuxAccount = userobj.value("linuxAccount").toArray();
    for (int i = 0; i < linuxAccount.size(); i++) {
        auto obj = linuxAccount[i].toObject();
        if (obj.value("machine").toString() == machine && obj.value("account").toString() == account) {
            linuxAccount.removeAt(i);
            break;
        }
    }
    userobj.insert("linuxAccount", linuxAccount);
    this->_users.insert(user, userobj);
    emit this->dataChanged();
}

void DataBase::setEmail(QString user, QString email) {
    if(!this->_users.contains(user)) {
        return;
    }
    auto userobj = this->_users.value(user).toObject();
    userobj.insert("email", email);
    this->_users.insert(user, userobj);
    emit this->dataChanged();
}

void DataBase::setPassword(QString user, QString password) {
    if(!this->_users.contains(user)) {
        return;
    }
    auto userobj = this->_users.value(user).toObject();
    userobj.insert("password", password);
    this->_users.insert(user, userobj);
    emit this->dataChanged();
}

QString DataBase::getPassword(QString user) {
    if(!this->_users.contains(user)) {
        return "";
    }
    auto userobj = this->_users.value(user).toObject();
    return userobj.value("password").toString();
}

QString DataBase::getEmail(QString user) {
    if(!this->_users.contains(user)) {
        return "";
    }
    auto userobj = this->_users.value(user).toObject();
    return userobj.value("email").toString();
}

QList<QString> DataBase::getLinuxAccount(QString user, QString machine) {
    QList<QString> ret;
    if(!this->_users.contains(user)) {
        return ret;
    }
    auto userobj = this->_users.value(user).toObject();
    auto linuxAccount = userobj.value("linuxAccount").toArray();
    for (int i = 0; i < linuxAccount.size(); i++) {
        auto obj = linuxAccount[i].toObject();
        if (obj.value("machine").toString() == machine) {
            ret.append(obj.value("account").toString());
        }
    }
    return ret;
}

QList<QPair<QString, QString>> DataBase::getLinuxAccount(QString user) {
    QList<QPair<QString, QString>> ret;
    if(!this->_users.contains(user)) {
        return ret;
    }
    auto userobj = this->_users.value(user).toObject();
    auto linuxAccount = userobj.value("linuxAccount").toArray();
    for (int i = 0; i < linuxAccount.size(); i++) {
        auto obj = linuxAccount[i].toObject();
        ret.append(QPair<QString, QString>{obj.value("machine").toString(), obj.value("account").toString()});
    }
    return ret;
}

bool DataBase::hasUser(QString user) {
    return this->_users.contains(user);
}
