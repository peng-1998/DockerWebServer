#pragma once
#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
namespace tools
{
    void __await(std::function<bool()> &&function)
    {
        QAbstractEventDispatcher *eventDispatcher = QThread::currentThread()->eventDispatcher();
        if (eventDispatcher->hasPendingEvents())
        {
            while (!function())
            {
                eventDispatcher->processEvents(QEventLoop::AllEvents, 100);
            }
        }
        else{
            while (!function())
            {
                QThread::sleep(100);
            }
        }
    }
}
#define await(x) tools::__await([&]() { return x; })