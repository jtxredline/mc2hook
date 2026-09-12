#include "PrinterHandler.h"
#include <core/output.h>

void PrinterHandler::Install()
{
    datOutput::SetOutputMask(0xFFFFFFFF); // Lets view all those sweet output messages!
}
