#pragma once
#include <mc2hook/mc2hook.h>
#include <veh_base/factory.h>

class vehEntity;
class mcCar;
class aiOpponent;

class aiOpponentFactory : vehFactory
{
public:
	mcCar* m_Car;
	int m_Idx;
	aiOpponent* m_Owner;
	char buffer[48];

public:
	aiOpponentFactory::aiOpponentFactory(const char* carName, int idx, aiOpponent* owner);

	vehEntity* Construct();
	vehEntity* GetEntity() const;

	void MakeEntity();
	void MakeSim();
	void MakeAIInput();
	void MakeModel();
	void MakeDamage();
	void MakeAudio();
	void MakeGyro();
	void MakeDriver();
};
