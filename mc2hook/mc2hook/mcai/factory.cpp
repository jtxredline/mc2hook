#include "factory.h"
#include <memory/memory.h>
#include <veh_base/entity.h>
#include <mccar/carsim.h>
//#include <veh_dyna/aero.h>
#include <veh_dyna/automgr.h>
#include <mccar/car.h>
#include <age/vehicle/nitro.h>
#include <mccar/carSSTurbo.h>
#include <age/vehicle/aiinfo.h>
#include <parse/parse.h>
#include <physics/archetype.h>
//#include <veh_base/input.h>
#include <veh_base/model.h>
#include <data/args.h>
#include <physics/collider.h>
#include <age/physics/ph_some_inst_parent.h>
#include <veh_dyna/gyro.h>

#include <core/output.h> //

void aiOpponentFactory::MakeEntity()
{
    //hook::Thunk<0x4BF2C0>::Call<void>(this); // Call original

    vehEntity* entity = age_new vehEntity();

    if (entity)
        m_Car = &entity->m_Car;
    else
        m_Car = nullptr;
}

void aiOpponentFactory::MakeSim()
{
    hook::Thunk<0x4BF300>::Call<void>(this); // Call original
    //return;    

    //vehEntity* entity = GetEntity();

    //mcCarSim* sim = age_new mcCarSim();

    //entity->m_Car.m_CarSim = sim;

    //sim->MakeCollider(m_CarName, entity);
    //sim->MakeAero(m_CarName);
    //sim->MakeFluid(m_CarName);
    //sim->MakeTransmission(m_CarName);
    //sim->MakeEngine(m_CarName);
    //sim->MakeWheels(m_CarName);
    //sim->MakeDrivetrains(m_CarName);
    //sim->MakeAxles(m_CarName);
    //sim->MakeSuspensions(m_CarName);

    //sim->m_Nitro = age_new vehNitro();
    //sim->m_Nitro->Init(-1, entity, m_CarName);

    //sim->m_SSTurbo = age_new mcCarSSTurbo();
    //sim->m_SSTurbo->Init(-1, entity, m_CarName);

    //sim->m_AIInfo = age_new carAIInfo();

    //sim->sub_569A80(m_CarName); // Some Load
    //sim->sub_575060(&datParser::dword_8600B0); // ?

    //sim->dword_B4 = 0x15; //
    //sim->sub_4D2F60();

    //phArchetype* archetype = entity->m_PhysInst.m_Archetype;
    //if (archetype)
    //{
    //    archetype->SetTypeFlag(64, 1);
    //    archetype->SetTypeFlag(1024, 1);
    //}

    //// HIWORD(entity->m_PhysInst.dword_08) |= (0x10 * LOWORD(this->m_Idx) + 0x10) | 8;
    //uint16_t& physFlags = reinterpret_cast<uint16_t*>(&entity->m_PhysInst.dword_08)[1];
    //physFlags |= (static_cast<uint16_t>(m_Idx) * 0x10 + 0x10) | 8;

    //// LOBYTE(entity->m_Car.dword_3c) = this->m_Idx;
    //// BYTE1(entity->m_Car.dword_3c) = 1;
    //uint8_t* carFlags = reinterpret_cast<uint8_t*>(&entity->m_Car.dword_3c);
    //carFlags[0] = static_cast<uint8_t>(m_Idx);
    //carFlags[1] = 1;
}

void aiOpponentFactory::MakeModel()
{
    //hook::Thunk<0x4BF490>::Call<void>(this); // Call original

    vehEntity* entity = GetEntity();
    vehModel* model = age_new vehModel();

    datArgParser::Get("nohighlods");

    bool isBike = (entity->m_Car.m_CarSim->m_NumWheels == 2);

    model->Init(m_CarName, &entity->m_Car.m_CarSim->m_Collider->m_SomeInstParent->m_SomeInstParentTransform, entity->m_Car.m_CarSim, false, false, isBike);

    // This makes car models appear in the world
    hook::Thunk<0x5178A0>::Call<void>(&model->dword_0c);

    entity->m_Car.m_Model = model;
}

void aiOpponentFactory::MakeDamage()
{
    hook::Thunk<0x4BF580>::Call<void>(this); // Call original

    //vehEntity* entity = GetEntity();
}

void aiOpponentFactory::MakeGyro()
{
    //hook::Thunk<0x4BF720>::Call<void>(this); // Call original

    vehEntity* entity = GetEntity();

    if (entity->m_Car.m_CarSim->m_NumWheels == 2)
    {
        vehBikeGyro* gyro = age_new vehBikeGyro();
        gyro->Init(entity->m_Car.m_CarSim, m_CarName);
        gyro->dword_18 = (gyro->dword_18 & 0xFFE6FFFF) | 0x180000;
        gyro->m_Input = entity->m_Car.m_Input;
        entity->m_Car.m_Gyro = gyro;
    }
    else
    {
        vehGyro* gyro = age_new vehGyro();
        gyro->Init(entity->m_Car.m_CarSim, m_CarName);
        gyro->dword_18 = (gyro->dword_18 & 0xFFE6FFFF) | 0x100000;
        gyro->m_Input = entity->m_Car.m_Input;
        entity->m_Car.m_Gyro = gyro;
    }
}

aiOpponentFactory::aiOpponentFactory(const char* carName, int idx, void* owner)
{
    hook::Thunk<0x4BF290>::Call<void>(this, carName, idx, owner); // Call original
}

vehEntity* aiOpponentFactory::Construct()
{
    //return hook::Thunk<0x4BF910>::Call<vehEntity*>(this); // Call original

    vehAutoMgr* vehMgr = vehAutoMgr::GetInstance();

    MakeEntity();
    MakeSim();
    MakeAIInput();
    MakeModel();
    MakeDamage(); // ?
    MakeAudio();

    vehEntity* entity = GetEntity();

    hook::Thunk<0x4BF850>::Call<void>(this);
    MakeGyro();
    MakeDriver();
    hook::Thunk<0x4BF7F0>::Call<void>(this); // Some MakeDamage

    if (entity) vehMgr->AddEntry(m_Car);
    else vehMgr->AddEntry(nullptr);

    return entity;
}

vehEntity* aiOpponentFactory::GetEntity() const
{
    if (m_Car)
        return (vehEntity*)&m_Car[0xFFFFFFFF]; // m_Car - 0x40
    else
        return nullptr;
}
