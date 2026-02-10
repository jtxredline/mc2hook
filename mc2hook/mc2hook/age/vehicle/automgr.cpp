#include "automgr.h"

declfield(vehAutoMgr::Instance)(0x6C523C);

vehAutoMgr* vehAutoMgr::GetInstance()
{
    return vehAutoMgr::Instance.get();
}
