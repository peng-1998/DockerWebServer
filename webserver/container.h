#ifndef CONTAINER_H
#define CONTAINER_H
#include <QPair>
#include <QList>
#include <QString>
class Container
{
public:
    Container();
    Container(const QString &name, const QList<QPair<int, int> > &ports, const QString &image, bool stopped);

    QString name() const;

    QList<QPair<int, int> > ports() const;

    QString image() const;

    bool stopped() const;

private:
    QString _name;
    QList<QPair<int,int>> _ports;
    QString _image;
    bool _stopped;
};

#endif // CONTAINER_H
