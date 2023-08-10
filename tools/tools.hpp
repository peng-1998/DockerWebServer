#pragma once
#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>

constexpr unsigned long await_timeout = 100l;

namespace tools
{
    void __await(std::function<bool()> &&function)
    {
        QAbstractEventDispatcher *eventDispatcher = QThread::currentThread()->eventDispatcher();
        if (eventDispatcher->hasPendingEvents())
        {
            while (!function())
            {
                eventDispatcher->processEvents(QEventLoop::AllEvents, await_timeout);
            }
        }
        else
        {
            while (!function())
            {
                QThread::sleep(await_timeout);
            }
        }
    }
}
#define await(x) tools::__await([&]() { return x; })