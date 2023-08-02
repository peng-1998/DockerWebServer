#pragma once
#include <QEventLoop>
#define await(x) tools::await([&](){ return x; })
namespace tools
{
    void await(std::function<bool()> &&function){
        while(!function()){
            QEventLoop::processEvents(QEventLoop::AllEvents, 500);
        }
    }
}