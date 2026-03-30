#include "device.h"
#include <mc2hook\mc2hook.h>
#include <age/input/input.h>

declfield(ioDeviceWrapper::g_InputMap)(0x695988);

float ioDeviceWrapper::GetInput(int axisIdx)
{
    int inputType = (int)g_InputMap[2 * axisIdx];

    uint8_t valueByte =
        this->inputData[4 * axisIdx] ^
        this->inputData[4 * axisIdx + 3];

    // Controller / analog input
    if (inputType == 9 || inputType == 0xB || inputType == 1)
    {
        float value = (valueByte - 127.5f) * 0.0078431377f;

        float result;

        if (value < -1.0f)
            result = -1.0f;
        else if (value > 1.0f)
            result = 1.0f;
        else
            result = value;

        return ioInput::ioAddDeadZone(result, 0.1f);
    }

    // Keyboard / mouse (digital)
    return (valueByte > 0x7F) ? 1.0f : 0.0f;
}

bool ioDeviceWrapper::sub_467E90(int a2)
{
    return hook::Thunk<0x467E90>::Call<bool>(this, a2);
}

void ioDeviceWrapper::sub_467DF0()
{
    hook::Thunk<0x467DF0>::Call<void>(this);
}

bool ioDeviceWrapper::sub_467FA0()
{
    return hook::Thunk<0x467FA0>::Call<bool>(this);
}
