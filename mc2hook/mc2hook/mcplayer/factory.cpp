#include "factory.h"
#include <age/vehicle/entity.h>
#include <mccar/carsim.h>
#include <mccar/caraero.h>
#include <veh_dyna/automgr.h>
#include <age/vehicle/car.h>
#include <age/vehicle/nitro.h>
#include <age/vehicle/carSSTurbo.h>
#include <age/vehicle/aiinfo.h>
#include <age/data/parse.h>
#include <age/physics/archetype.h>
#include <mcplayer/input.h>
#include <veh_base/model.h>
#include <age/memory/memory.h>
#include <age/data/args.h>
#include <age/physics/phcollider.h>
#include <age/physics/ph_some_inst_parent.h>
#include <age/vehicle/gyro.h>

#include <age/core/output.h> //

void mcPlayerFactory::MakeEntity()
{
    //hook::Thunk<0x46BEE0>::Call<void>(this); // Call original

    vehEntity* entity = age_new vehEntity();

    if (entity)
        m_Car = &entity->m_Car;
    else
        m_Car = nullptr;
}

void mcPlayerFactory::MakeSim()
{
    //hook::Thunk<0x46BFA0>::Call<void>(this); // Call original

    vehEntity* entity = GetEntity();

    mcCarSim* sim = age_new mcCarSim();

    sim->MakeCollider(m_CarName, entity);
    sim->MakeAero(m_CarName);
    sim->MakeFluid(m_CarName);
    sim->MakeTransmission(m_CarName);
    sim->MakeEngine(m_CarName);
    sim->MakeWheels(m_CarName);
    sim->MakeDrivetrains(m_CarName);
    sim->MakeAxles(m_CarName);
    sim->MakeSuspensions(m_CarName);

    entity->m_Car.m_CarSim = sim;

    vehNitro* nitro = age_new vehNitro();
    nitro->Init(m_SomePlayerID, entity, m_CarName);
    sim->m_Nitro = nitro;

    mcCarSSTurbo* sst = age_new mcCarSSTurbo();
    sst->Init(m_SomePlayerID, entity, m_CarName);
    sim->m_SSTurbo = sst;

    sim->m_AIInfo = age_new carAIInfo();

    sim->sub_569A80(m_CarName); // mcPowerUpMgr::Load ?
    sim->SetResetPos(datParser::dword_8600B0);

    sim->dword_B4 = 0x15; //
    sim->sub_4D2F60();

    phArchetype* archetype = entity->m_PhysInst.m_Archetype;
    if (archetype)
    {
        archetype->SetTypeFlag(64, 1);
        archetype->SetTypeFlag(0, 1);
    }

    // HIWORD(Entity->m_PhysInst.dword_08) |= 0x10 * LOWORD(this->dword_4c) + 0x10;
    uint16_t& physFlags = reinterpret_cast<uint16_t*>(&entity->m_PhysInst.dword_08)[1];
    physFlags |= (static_cast<uint16_t>(m_SomePlayerID) * 0x10) + 0x10;

    // LOBYTE(Entity->m_Car.dword_3c) = this->dword_4c;
    // BYTE1(Entity->m_Car.dword_3c) = 0;
    uint8_t* carFlags = reinterpret_cast<uint8_t*>(&entity->m_Car.dword_3c);
    carFlags[0] = static_cast<uint8_t>(m_SomePlayerID);
    carFlags[1] = 0;
}

void mcPlayerFactory::MakePlayerInput()
{
    //hook::Thunk<0x46BF20>::Call<void>(this); // Call original

    vehEntity* entity = GetEntity();

    if (m_PlayerID == -1)
    {
        entity->m_Car.m_Input = nullptr;
        return;
    }

    mcPlayerInput* input = age_new mcPlayerInput(0, 1, 0);

    input->Init(entity, m_CarName);
    input->SetIndex(m_PlayerID);
    input->SetStyle(m_PlayerID);

    entity->m_Car.m_Input = input;
}

void mcPlayerFactory::MakeModel()
{ 
    hook::Thunk<0x46C150>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeDamage()
{
    hook::Thunk<0x46C280>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeStuck()
{
    hook::Thunk<0x46C2D0>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeGyro()
{
    hook::Thunk<0x46C320>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeDriver()
{
    hook::Thunk<0x46C3D0>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeFeedback()
{
    hook::Thunk<0x46C4A0>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeTracks()
{
    hook::Thunk<0x46C7C0>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeParticles()
{
    hook::Thunk<0x46C670>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakePickups()
{
    hook::Thunk<0x46C750>::Call<void>(this); // Call original
}
void mcPlayerFactory::MakeAudio()
{
    hook::Thunk<0x46C5A0>::Call<void>(this); // Call original
}

mcPlayerFactory::mcPlayerFactory(const char* carName, int idx, void* owner)
{
    hook::Thunk<0x46BDC0>::Call<void>(this, carName, idx, owner); // Call original
}

vehEntity* mcPlayerFactory::Construct()
{
    //return hook::Thunk<0x46BE10>::Call<vehEntity*>(this); // Call original
    
    //int ghost = hook::Thunk<0x46BE10>::Call<int>(this); // This creates a second car already

    Printf("mcPlayerFactory = %p", this);

    MakeEntity();
    MakeSim();
    MakePlayerInput();
    MakeModel();
    MakeDamage();
    MakeStuck();
    MakeGyro();
    MakeDriver();
    MakeFeedback();
    MakeTracks();
    MakeParticles();
    MakePickups();
    MakeAudio();

    vehEntity* entity = GetEntity();
    mcCarSim* sim = entity->m_Car.m_CarSim;

    if (sim->m_NumWheels == 2) // If bike
    {
        mcCarAero* aero = sim->m_Aero;
        vehInput* input = entity->m_Car.m_Input;

        aero->Init(sim, input);
    }

    vehAutoMgr* mgr = vehAutoMgr::Instance;
    if (mgr) mgr->AddEntry(&entity->m_Car);

    return entity;
}

vehEntity* mcPlayerFactory::GetEntity() const
{
    if (m_Car)
        return (vehEntity*) &m_Car[0xFFFFFFFF]; // m_Car - 0x40
    else
        return nullptr;
}
