#include "factory.h"
#include <age/memory/memory.h>
#include <age/vehicle/entity.h>
#include <mccar/carsim.h>
#include <age/vehicle/automgr.h>
#include <age/vehicle/car.h>
#include <age/vehicle/nitro.h>
#include <age/vehicle/carSSTurbo.h>
#include <age/vehicle/aiinfo.h>
#include <age/data/parse.h>
#include <age/physics/archetype.h>
#include <mcai/input.h>
#include <veh_base/model.h>
#include <age/data/args.h>
#include <age/physics/phcollider.h>
#include <age/physics/ph_some_inst_parent.h>
#include <age/vehicle/gyro.h>
#include <mcai/damage.h>
#include <mccar/caraudio.h>
#include <snd_control/audiomgr.h>
#include <veh_base/model.h>
#include <mccar/character.h>
#include <mccar/cardriver.h>

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
    //hook::Thunk<0x4BF300>::Call<void>(this); // Call original

    vehEntity* entity = GetEntity();

    mcCarSim* sim = age_new mcCarSim();

    entity->m_Car.m_CarSim = sim;

    sim->MakeCollider(m_CarName, entity);
    sim->MakeAero(m_CarName);
    sim->MakeFluid(m_CarName);
    sim->MakeTransmission(m_CarName);
    sim->MakeEngine(m_CarName);
    sim->MakeWheels(m_CarName);
    sim->MakeDrivetrains(m_CarName);
    sim->MakeAxles(m_CarName);
    sim->MakeSuspensions(m_CarName);

    sim->m_Nitro = age_new vehNitro();
    sim->m_Nitro->Init(-1, entity, m_CarName);

    sim->m_SSTurbo = age_new mcCarSSTurbo();
    sim->m_SSTurbo->Init(-1, entity, m_CarName);

    sim->m_AIInfo = age_new carAIInfo();

    sim->sub_569A80(m_CarName); // Some Load
    sim->sub_575060(&datParser::dword_8600B0); // ?

    sim->dword_B4 = 0x15; //
    sim->sub_4D2F60();

    phArchetype* archetype = entity->m_PhysInst.m_Archetype;
    if (archetype)
    {
        archetype->SetTypeFlag(64, 1);
        archetype->SetTypeFlag(1024, 1);
    }

    // HIWORD(entity->m_PhysInst.dword_08) |= (0x10 * LOWORD(this->m_Idx) + 0x10) | 8;
    uint16_t& physFlags = reinterpret_cast<uint16_t*>(&entity->m_PhysInst.dword_08)[1];
    physFlags |= (static_cast<uint16_t>(m_Idx) * 0x10 + 0x10) | 8;

    // LOBYTE(entity->m_Car.dword_3c) = this->m_Idx;
    // BYTE1(entity->m_Car.dword_3c) = 1;
    uint8_t* carFlags = reinterpret_cast<uint8_t*>(&entity->m_Car.dword_3c);
    carFlags[0] = static_cast<uint8_t>(m_Idx);
    carFlags[1] = 1;
}

void aiOpponentFactory::MakeAIInput()
{
    //hook::Thunk<0x4BF530>::Call<void>(this); // Call original

    vehEntity* entity = GetEntity();
    m_Car->m_Input = age_new aiInput(entity);
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
    //hook::Thunk<0x4BF580>::Call<void>(this); // Call original

    vehEntity* entity = GetEntity();

    aiDamage* damage = age_new aiDamage(m_Owner);
    damage->Init(entity, m_CarName);

    entity->m_Car.m_Damage = damage;
    entity->m_Car.m_CarSim->m_Damage = damage;
}

void aiOpponentFactory::MakeAudio()
{
    //hook::Thunk<0x4BF5E0>::Call<void>(this); // Call original

    vehEntity* entity = GetEntity();
    mcCarAudio* audio = age_new mcCarAudio(true);
    audio->Init(entity, m_CarName, sndAudioManager::smInstance->m_ControlVolumeGroup);
    reinterpret_cast<uint8_t*>(&audio->dword_39c)[1] = static_cast<uint8_t>(m_Idx) | 0x80;
    entity->m_Car.m_Audio = audio;
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

void aiOpponentFactory::MakeDriver()
{
    //hook::Thunk<0x4BF650>::Call<void>(this); // Call original

    vehEntity* entity = GetEntity();
    mcCarSim* carSim = entity->m_Car.m_CarSim;
    vehModel* model = entity->m_Car.m_Model;

    if (carSim->m_NumWheels == 2) // Make bike rider
    {
        mcCharacter* character = age_new mcCharacter();
        character->Init(carSim, entity->m_Car.m_Input, m_CarName, "rider", 0, false);

        character->dword_288 = model->dword_64;
        character->sub_4D8F30(model->dword_64);

        model->m_Driver = character;
        entity->m_Car.m_Driver = character;
    }
    else
    {
        mcCarDriver* driver = age_new mcCarDriver();
        driver->Init(carSim, m_CarName);

        model->m_Driver = driver;
        entity->m_Car.m_Driver = driver;
    }
}

aiOpponentFactory::aiOpponentFactory(const char* carName, int idx, aiOpponent* owner)
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
    MakeDamage();
    MakeAudio();

    vehEntity* entity = GetEntity();

    hook::Thunk<0x4BF850>::Call<void>(this); // MakeTracks
    MakeGyro();
    MakeDriver();
    hook::Thunk<0x4BF7F0>::Call<void>(this); // MakePickups

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
