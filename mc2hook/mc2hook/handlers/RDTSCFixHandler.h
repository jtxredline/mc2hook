#pragma once
#include <mc2hook\mc2hook.h>

class RDTSCFixHandler
{
public:
    static uint32_t RTDSC_QPC_Hook();
    static void Install();
};