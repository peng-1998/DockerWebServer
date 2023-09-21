#include "globaldata.h"

GlobalData& GlobalData::instance()
{
    static GlobalData globaldata_instance;
    return globaldata_instance;
}