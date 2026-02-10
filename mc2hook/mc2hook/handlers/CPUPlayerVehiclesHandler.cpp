#include "CPUPlayerVehiclesHandler.h"

void CPUPlayerVehiclesHandler::Install()
{
    bool aiUsePlayerModels = HookConfig::GetBool("Model", "AIUsePlayerModels", true);
    if (aiUsePlayerModels)
    {
        // Force player models
        uint8_t p1[5] = { 0xBB, 0x01, 0x00, 0x00, 0x00 }; // mov ebx, 1
        mem::write(0x4E524E, p1[0], p1[1], p1[2], p1[3], p1[4]);
        mem::nop(0x4E524E + 5, 2); // NOP padding

        // Force use damage models
        uint8_t p2[5] = { 0xBA, 0x01, 0x00, 0x00, 0x00 }; // a4 = 1 (damage model?)
        mem::write(0x4E52BD, p2[0], p2[1], p2[2], p2[3], p2[4]);
        mem::nop(0x4E52BD + 5, 2);

        uint8_t p3[5] = { 0xB8, 0x01, 0x00, 0x00, 0x00 }; // a5 = 1 (high LOD?)
        mem::write(0x4E52C4, p3[0], p3[1], p3[2], p3[3], p3[4]);
        mem::nop(0x4E52C4 + 5, 2);
    }
}