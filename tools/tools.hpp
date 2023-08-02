#pragma once
#include <QEventLoop>
#include <QCoreApplication>
namespace tools
{
    void __await(std::function<bool()> &&function){
        while(!function()){
            QCoreApplication::processEvents(QEventLoop::AllEvents, 500);
        }
    }
}
#define await(x) tools::__await([&](){ return x; })