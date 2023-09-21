#include "globaldata.h"

GlobalData& GlobalData::instance()
{
    static GlobalData _instance;
    return _instance;
}