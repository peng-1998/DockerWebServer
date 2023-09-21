#pragma once
#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <QAbstractEventDispatcher>

constexpr unsigned long await_timeout = 100l;

namespace tools
{
    template <typename Func>
    void __await(Func &&func)
    {
        static_assert(std::is_same_v<bool, decltype(func())>, "await() requires a function returning bool");
        QAbstractEventDispatcher *eventDispatcher = QThread::currentThread()->eventDispatcher();
        if (eventDispatcher)
        {
            while (!func())
            {
                eventDispatcher->processEvents(QEventLoop::AllEvents);
            }
        }
        else
        {
            while (!func())
            {
                QThread::sleep(await_timeout);
            }
        }
    }
}
#define await(x) tools::__await([&]() { return x; })