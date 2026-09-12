#include "InputHandler.h"
#include <data\args.h>
#include <input\input.h>

void InputHandler::Install()
{
    if (datArgParser::Get("nojoy"))
    {
        ioInput::bUseJoystick.set(false);
    }
    if (datArgParser::Get("nokb"))
    {
        ioInput::bUseKeyboard.set(false);
    }
}
