#include "ghost.h"
#include <mc2hook/mc2hook.h>
#include <memory/memory.h>
//#include <mcplayer/factory.h>
#include <veh_base/entity.h>
#include <mccar/carsim.h>
#include <veh_base/model.h>
#include <age/vehicle/nitro.h>
#include <mccar/carSSTurbo.h>
#include <age/vehicle/aiinfo.h>
#include <parse/parse.h>
#include <physics/archetype.h>
#include <data/args.h>
#include <physics/collider.h>

#include <core/output.h> //

bool mcGhostCar::Spawn(const char* carName)
{
    //// TODO: Check if it's possible to have a mcGhostFactory,
    //// creating only what's needed,
    //// and possibly having a custom Update in vehAutoMgr/vehManager.

    //if (m_Entity)
    //    return false;

    //vehFactory factory;

    //factory.Build(carName, 1, nullptr); // Setting idx to higher than 0 enables collisions again (when other AIs are present)

    //m_Entity = factory.Create();

    //return m_Entity != nullptr;

    Destroy();

    m_Entity = age_new vehEntity();

    MakeSim(carName);
    //MakeModel(carName);

    return m_Entity != nullptr;
}

void mcGhostCar::MakeSim(const char* carName)
{
    //mcCarSim* sim = age_new mcCarSim();

    //m_Entity->m_Car.m_CarSim = sim;

    //sim->MakeCollider(carName, m_Entity);
    //sim->MakeAero(carName);
    //sim->MakeFluid(carName);
    //sim->MakeTransmission(carName);
    //sim->MakeEngine(carName);
    //sim->MakeWheels(carName);
    //sim->MakeDrivetrains(carName);
    //sim->MakeAxles(carName);
    //sim->MakeSuspensions(carName);

    //sim->m_Nitro = age_new vehNitro();
    //sim->m_Nitro->Init(-1, m_Entity, carName);

    //sim->m_SSTurbo = age_new mcCarSSTurbo();
    //sim->m_SSTurbo->Init(-1, m_Entity, carName);

    //sim->m_AIInfo = age_new carAIInfo();

    //sim->sub_569A80(carName); // Some Load
    //sim->sub_575060(&datParser::dword_8600B0); // ?

    //sim->field_B4 = 0x15; //
    //sim->sub_4D2F60();

    //phArchetype* archetype = m_Entity->m_PhysInst.m_Archetype;
    //if (archetype)
    //{
    //    archetype->SetTypeFlag(64, 1);
    //    archetype->SetTypeFlag(1024, 1);
    //}

    //// HIWORD(entity->m_PhysInst.dword_08) |= (0x10 * LOWORD(this->m_Idx) + 0x10) | 8;
    //uint16_t& physFlags = reinterpret_cast<uint16_t*>(&m_Entity->m_PhysInst.dword_08)[1];
    ////physFlags |= (static_cast<uint16_t>(m_Idx) * 0x10 + 0x10) | 8;

    //// LOBYTE(entity->m_Car.dword_3c) = this->m_Idx;
    //// BYTE1(entity->m_Car.dword_3c) = 1;
    //uint8_t* carFlags = reinterpret_cast<uint8_t*>(&m_Entity->m_Car.dword_3c);
    ////carFlags[0] = static_cast<uint8_t>(m_Idx);
    ////carFlags[1] = 1;
}

void mcGhostCar::MakeModel(const char* carName)
{
    vehModel* model = age_new vehModel();

    datArgParser::Get("nohighlods");

    bool isBike = (m_Entity->m_Car.m_CarSim->m_NumWheels == 2);

    model->Init(carName, &m_Entity->m_Car.m_CarSim->m_Collider->m_SomeInstParent->m_SomeInstParentTransform, m_Entity->m_Car.m_CarSim, false, false, isBike);

    // This makes car models appear in the world
    hook::Thunk<0x5178A0>::Call<void>(&model->dword_0c);

    m_Entity->m_Car.m_Model = model;
}

void mcGhostCar::Update()
{
    Printf("mcGhostCar::Update\n");
    //hook::Thunk<0x4CA4D0>::Call<void>(&m_Entity->m_Car.m_Model); // vehModel::Update
}

void mcGhostCar::Destroy()//(bool a2)
{
    if (!m_Entity) return;

    m_Entity->Delete(true);
    m_Entity = nullptr;
}
