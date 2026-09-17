#include "chassis.h"
#include <age/memory/memory.h>
#include <age/physics/phcollider.h>
#include <age/physics/phinertia.h>
#include <age/physics/bound.h>
#include <age/core/output.h>
#include <age/physics/archmgr.h>
#include <age/physics/archetype.h>
#include <age/vehicle/entity.h>
#include <age/physics/phlevel.h>
#include <mccar/caraero.h>
#include <mccar/carfluid.h>
#include <veh_dyna/transmission.h>
#include <veh_dyna/engine.h>

int vehChassis::OnGround()
{
    //return hook::Thunk<0x56A000>::Call<int>(this); // Call original

    int numWheelsOnGround = 0;

    for (int i = 0; i < m_NumWheels; i++)
    {
        if (m_Wheels[i]->m_OnGround)
            numWheelsOnGround++;
    }
    return numWheelsOnGround;
}

int vehChassis::BottomedOut()
{
    int numWheelsBottomedOut = 0;

    for (int i = 0; i < m_NumWheels; i++)
    {
        if (m_Wheels[i]->m_BottomedOut)
            numWheelsBottomedOut++;
    }
    return numWheelsBottomedOut;
}

void vehChassis::MakeCollider(const char* carName, vehEntity* entity)
{
    //hook::Thunk<0x569010>::Call<void>(this, carName, entity); // Call original

    // Create collider
    m_Collider = age_new phCollider();

    if (m_Collider)
    {
        m_Collider->m_Vtable = &phCollider::SomeVtable;
        m_Collider->m_CarSim = (mcCarSim*)this;
    }

    // Load vehicle bounds
    char boundName[64];
    sprintf(boundName, "%s_bound", carName);

    phBound* bound = phBound::Load(boundName);

    if (bound)
    {
        m_Collider->m_Bound = bound;
        m_Size.X = bound->dword_14 - bound->dword_08;
        m_Size.Y = bound->dword_18 - bound->dword_0c;
        m_Size.Z = bound->dword_1c - bound->dword_10;
        m_Size.Y = bound->dword_18;
    }
    else Errorf("vehChassis::MakeCollider(): File %s not found", boundName);

    // Default inertia box
    if (m_InertiaBox.X == 0.0f && m_InertiaBox.Y == 0.0f && m_InertiaBox.Z == 0.0f)
    {
        m_InertiaBox = m_Size * 1.25f;
        m_InertiaScale = Vector3(1.25f, 1.25f, 1.25f);
    }

    // Create ICS
    phInertialCS* ics = age_new phInertialCS();

    ics->m_MaxAngVelocity = Vector3(6.2831855f, 6.2831855f, 6.2831855f); // 2PI
    ics->m_MaxVelocity = 111.75f;

    // Read max velocity from .ini
    bool speedrunMode = HookConfig::GetBool("General", "SpeedrunMode", false);
    float maxVelocity = HookConfig::GetFloat("Physics", "MaxVelocity", 111.75 * 2.237); // In mph format
    if (!speedrunMode) ics->m_MaxVelocity = maxVelocity / 2.237; // mph to ms

    ics->InitBoxMass(m_Mass, m_InertiaBox.X, m_InertiaBox.Y, m_InertiaBox.Z);
    ics->Zero();

    m_Collider->m_ICS = ics;

    // Create physics archetype
    if (!phArchetypeMgr::Instance) phArchetypeMgr::CreateInstance(0x190);

    char s[60];
    phArchetype* archetype = phArchetypeMgr::Instance->RegisterArchetype(bound, s, false, 1, phArchetype::dword_674060);

    archetype->SetMass(ics->m_Mass);
    archetype->SetAngInertia(ics->m_AngInertia);

    archetype->SetTypeFlag(64, 1);

    entity->m_PhysInst.SetArchetype(archetype);
    entity->m_PhysInst.m_Transform = Matrix34::I;

    m_Collider->Init(entity, ics, 0);

    // Add collider to physics level
    if (phLevel::Instance) phLevel::Instance->AddActiveObject(m_Collider, false);
}

void vehChassis::MakeAero(const char* carName)
{
    //hook::Thunk<0x4D2490>::Call<void>(this, carName); // Call original

    m_Aero = age_new mcCarAero();

    if (m_Aero) m_Aero->m_CarSim = (mcCarSim*)this;
}

void vehChassis::MakeFluid(const char* carName)
{
    //hook::Thunk<0x4D24E0>::Call<void>(this, carName); // Call original

    m_Fluid = age_new mcCarFluid();

    if (m_Fluid)
    {
        phBound* bound = m_Collider->m_Bound;
        m_Fluid->Init((mcCarSim*)this, &bound->dword_08, &bound->dword_14);
    }
}

void vehChassis::MakeTransmission(const char* carName)
{
    m_Transmission = age_new vehTransmission();
}

void vehChassis::MakeEngine(const char* carName)
{
    //hook::Thunk<0x569320>::Call<void>(this, carName); // Call original

    m_Engine = age_new vehEngine();

    m_Engine->Init(this, carName, "engine");
}

void vehChassis::MakeWheels(const char* carName)
{
    hook::Thunk<0x56AAB0>::Call<void>(this, carName); // Call original
}

void vehChassis::MakeDrivetrains(const char* carName)
{
    hook::Thunk<0x5693C0>::Call<void>(this, carName); // Call original
}

void vehChassis::MakeAxles(const char* carName)
{
    hook::Thunk<0x5694B0>::Call<void>(this, carName); // Call original
}

void vehChassis::MakeSuspensions(const char* carName)
{
    hook::Thunk<0x5695E0>::Call<void>(this, carName); // Call original
}
