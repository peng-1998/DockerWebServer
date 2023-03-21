#include "container.h"

Container::Container()
{

}

QString Container::name() const
{
    return _name;
}

QList<QPair<int, int> > Container::ports() const
{
    return _ports;
}

QString Container::image() const
{
    return _image;
}

bool Container::stopped() const
{
    return _stopped;
}

Container::Container(const QString &name, const QList<QPair<int, int> > &ports, const QString &image, bool stopped) : _name(name),
    _ports(ports),
    _image(image),
    _stopped(stopped)
{}
