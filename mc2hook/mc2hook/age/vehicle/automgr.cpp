#include "automgr.h"
#include <age/core/output.h>
#include <age/vehicle/entity.h>

declfield(vehAutoMgr::Instance)(0x6C523C); // vehManager?

vehAutoMgr* vehAutoMgr::GetInstance()
{
    return vehAutoMgr::Instance.get();
}

int vehAutoMgr::ManagerAddEntry(mcCar* a2) // vehManager? // void?
{
    return hook::Thunk<0x4CE990>::Call<int>(this, a2);
}

