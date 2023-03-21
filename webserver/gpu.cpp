#include "gpu.h"

GPU::GPU()
{

}

QString GPU::type() const
{
    return _type;
}



int GPU::gpuid() const
{
    return _gpuid;
}



int GPU::max_memory() const
{
    return _max_memory;
}


int GPU::used_memory() const
{
    return _used_memory;
}



GPU::GPU(const QString &type, int gpuid, int max_memory, int used_memory) : _type(type),
    _gpuid(gpuid),
    _max_memory(max_memory),
    _used_memory(used_memory)
{}
