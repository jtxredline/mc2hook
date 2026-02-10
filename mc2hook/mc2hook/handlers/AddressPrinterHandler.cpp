#include "AddressPrinterHandler.h"
#include <age/core/output.h>
#include <age/input/keyboard.h>
#include <dinput.h>

// camBaseCS: 
// camTrackCS: 0x57B660, 0x651A8C
// gfxViewport:
// phInertialCS: 0x594EC0, 0x652AF4
// phInst:
// vehAero: 0x4E5480, 0x646710
// vehAxle:
// vehCarSim: 0x56A3E0, 0x644A74 Maybe
// vehDamage: 0x4CFAF0, 0x6446D4
// vehDrivetrain: 0x573CB0, 0x56C696 (cb)
// vehEngine: 0x56FBD0, 650F9C
// vehFluid:
// vehGyro:
// vehNitro:
// vehStuck:
// vehSuspension:
// vehTransmission:
// vehInput: 0x46B330, 0x63D0C4
// vehWheel: 0x56DB90, 0x650E30
// mcCar: 0x4D1710, 0x644980
// mcRaceStateDerived: 0x482580, 0x63EC04
// mcGameState: 0x4045E0, 0x62D970
// phLevelSimple: 0x58A0E0, 0x63E3F8 // broken still
// mcPVS: 0x524030, 0x517C87 //cb::call
// vehChassis: 0x56A3E0, 0x644A74 ??
// phCollider: 
// vehAutoMgr: 0x56C580, 0x4CEA74 //cb::call
// vehManager: 0x4CEA70, 0x644690

void AddressPrinterHandler::PrintAddress() {
    hook::Thunk<0x57B660>::Call<void>(this);

    if (ioKeyboard::GetKeyDown(DIK_LALT))
    {
        void* pt = getPtr<void*>(this, 0x0);
        Printf("camTrackCS: %p\n", pt);
    }
}

void AddressPrinterHandler::Install()
{   
    bool printAddress = HookConfig::GetBool("Debug", "PrintAddress", false);
    if (printAddress)
    {
        //InstallVTableHook("Print Address", &PrintAddress, { 0x651A8C });

        //InstallCallback("Address Printer", "Address Printer", &PrintAddress, { cb::call(0x4CEA74) });
    }
}
