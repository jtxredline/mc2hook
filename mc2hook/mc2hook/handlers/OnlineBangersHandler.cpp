#include "OnlineBangersHandler.h"
#include <age/data/args.h>

void OnlineBangersHandler::Install()
{
    bool onlineBangers = HookConfig::GetBool("Network", "OnlineBangers", false);
    if (onlineBangers) {
        //mem::write(0x4836A3 + 2, 0x000000);
        //mem::write(0x4836A3, static_cast<uint32_t>(0));
    }    
}
