#pragma once
#include <mc2hook\mc2hook.h>
#include <veh_base/factory.h>

class vehEntity;
class mcCar;

class mcPlayerFactory : public vehFactory // WIP
{
public:
	mcCar* m_Car;
	int m_Idx;
	int m_SomePlayerID;
	int m_PlayerID;
	char buffer[44];

public:
	mcPlayerFactory::mcPlayerFactory(const char* carName, int idx, void* owner);

	vehEntity* Construct();
	vehEntity* GetEntity() const;

	void MakeEntity();
	void MakeSim();
	void MakePlayerInput();
	void MakeModel();
	void MakeDamage();
	void MakeStuck();
	void MakeGyro();
	void MakeDriver();
	void MakeFeedback();
	void MakeTracks();
	void MakeParticles();
	void MakePickups();
	void MakeAudio();
};
