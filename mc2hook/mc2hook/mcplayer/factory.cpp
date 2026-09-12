#include "factory.h"
#include <veh_base/entity.h>
#include <mccar/carsim.h>
#include <veh_dyna/aero.h>
#include <veh_dyna/automgr.h>
#include <mccar/car.h>
#include <age/vehicle/nitro.h>
#include <mccar/carSSTurbo.h>
#include <age/vehicle/aiinfo.h>
#include <parse/parse.h>
#include <physics/archetype.h>
#include <veh_base/input.h>
#include <veh_base/model.h>
#include <memory/memory.h>
#include <data/args.h>
#include <physics/collider.h>
#include <age/physics/ph_some_inst_parent.h>
#include <veh_dyna/gyro.h>

#include <core/output.h> //

//////////////////// mcPlayerFactory ////////////////////

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
    hook::Thunk<0x46BFA0>::Call<void>(this); // Call original
    //return;

    //vehEntity* entity = GetEntity();

    //mcCarSim* sim = age_new mcCarSim();

    //sim->MakeCollider(m_CarName, entity);
    //sim->MakeAero(m_CarName);
    //sim->MakeFluid(m_CarName);
    //sim->MakeTransmission(m_CarName);
    //sim->MakeEngine(m_CarName);
    //sim->MakeWheels(m_CarName);
    //sim->MakeDrivetrains(m_CarName);
    //sim->MakeAxles(m_CarName);
    //sim->MakeSuspensions(m_CarName);

    //entity->m_Car.m_CarSim = sim;

    //sim->m_Nitro = age_new vehNitro();
    //sim->m_Nitro->Init(this->dword_4c, entity, m_CarName);

    //sim->m_SSTurbo = age_new mcCarSSTurbo();
    //sim->m_SSTurbo->Init(this->dword_4c, entity, m_CarName);

    //sim->m_AIInfo = age_new carAIInfo();

    //sim->sub_569A80(m_CarName); // Some Load
    //sim->sub_575060(&datParser::dword_8600B0); // ?

    //sim->dword_B4 = 0x15; //
    //sim->sub_4D2F60();

    //phArchetype* archetype = entity->m_PhysInst.m_Archetype;
    //if (archetype)
    //{
    //    archetype->SetTypeFlag(64, 1);
    //    archetype->SetTypeFlag(2048, 1);
    //}

    //// HIWORD(Entity->m_PhysInst.dword_08) |= 0x10 * LOWORD(this->dword_4c) + 0x10;
    //uint16_t& physFlags = reinterpret_cast<uint16_t*>(&entity->m_PhysInst.dword_08)[1];
    //physFlags |= (static_cast<uint16_t>(dword_4c) * 0x10) + 0x10;

    //// LOBYTE(Entity->m_Car.dword_3c) = this->dword_4c;
    //// BYTE1(Entity->m_Car.dword_3c) = 0;
    //uint8_t* carFlags = reinterpret_cast<uint8_t*>(&entity->m_Car.dword_3c);
    //carFlags[0] = static_cast<uint8_t>(dword_4c);
    //carFlags[1] = 0;
}

void mcPlayerFactory::Build(const char* carName, int idx, void* owner)
{
    hook::Thunk<0x46BDC0>::Call<void>(this, carName, idx, owner); // Call original
}

vehEntity* mcPlayerFactory::Create()
{
	//int ghost = hook::Thunk<0x46BE10>::Call<int>(this); // This creates a second car already

    MakeEntity();
    MakeSim();
    MakePlayerInput();
    MaybeMakeModel();
    MakeDamage1();
    MakeStuck();
    MakeGyro();
    MakeDriver();
    MakeFeedback();
    MaybeMakeCamera();
    MaybeMakeWheelPtx();

    MakeDamage2();
    MaybeMakeAudio();

    vehEntity* entity = GetEntity();
    mcCarSim* sim = entity->m_Car.m_CarSim;

    if (sim->m_NumWheels == 2) // If bike
    {
        vehAero* aero = sim->m_Aero;
        vehInput* input = entity->m_Car.m_Input;

        aero->sub_4E5450(sim, input);
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

void mcPlayerFactory::MakePlayerInput()
{
    //hook::Thunk<0x46BF20>::Call<void>(this); // Call original

    vehEntity* entity = GetEntity();

    if (m_PlayerId == -1)
    {
        entity->m_Car.m_Input = nullptr;
        return;
    }

    vehInput* input = age_new vehInput(0, 1, 0);

    input->Init(entity, m_CarName);

    input->sub_46A3F0(m_PlayerId);
    input->sub_46A410(m_PlayerId);

    entity->m_Car.m_Input = input;
}
