#include "RDTSCFixHandler.h"

uint32_t RDTSCFixHandler::RTDSC_QPC_Hook()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.LowPart; // +(rand() % 16384);
}

void RDTSCFixHandler::Install()
{
    InstallCallback("RDTSC Fix Handler", "Replaces RDTSC high frequency timer with QPC (QueryPerformanceCounter).",
        &RTDSC_QPC_Hook, {
            cb::jmp(0x611C30)
        }
    );
}