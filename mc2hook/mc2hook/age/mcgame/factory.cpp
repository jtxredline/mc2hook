#include "factory.h"
#include <age/vehicle/entity.h>

#include <age/vehicle/carsim.h>
#include <age/physics/phcollider.h>
#include <age/vehicle/vehinput.h>
#include <age/vehicle/aero.h>
#include <age/vehicle/automgr.h>

vehEntity* mcPlayerFactory::MakeEntity()
{
	//int ghost = hook::Thunk<0x46BE10>::Call<int>(this); // This creates a second car already

    MakeInstance();
    MakeSim();
    MakeInput();
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
    vehCarSim* sim = entity->m_Car.m_CarSim;

    if (sim->m_NumWheels == 2) // If bike
    {
        vehAero* aero = sim->m_Aero;
        vehInput* input = entity->m_Car.m_Input;

        aero->sub_4E5450(sim, input);
    }

    vehAutoMgr* mgr = vehAutoMgr::Instance;
    if (mgr) mgr->ManagerAddEntry(&entity->m_Car);

    return entity;
}

vehEntity* mcPlayerFactory::GetEntity() const
{
	return hook::Thunk<0x46BE00>::Call<vehEntity*>(this); // Call original
}

void mcPlayerFactory::MakeInstance()
{
	hook::Thunk<0x46BEE0>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeSim()
{
	hook::Thunk<0x46BFA0>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeInput()
{
	hook::Thunk<0x46BF20>::Call<void>(this); // Call original
}

void mcPlayerFactory::MaybeMakeModel()
{
	hook::Thunk<0x46C150>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeDamage1()
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

void mcPlayerFactory::MakeDriver() // MakeRider or MakeSkeleton
{
	hook::Thunk<0x46C3D0>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeFeedback()
{
	hook::Thunk<0x46C4A0>::Call<void>(this); // Call original
}

void mcPlayerFactory::MaybeMakeCamera()
{
	hook::Thunk<0x46C7C0>::Call<void>(this); // Call original
}

void mcPlayerFactory::MaybeMakeWheelPtx()
{
	hook::Thunk<0x46C670>::Call<void>(this); // Call original
}

void mcPlayerFactory::MakeDamage2()
{
	hook::Thunk<0x46C750>::Call<void>(this); // Call original
}

void mcPlayerFactory::MaybeMakeAudio()
{
	hook::Thunk<0x46C5A0>::Call<void>(this); // Call original
}
