#pragma once
#include <mc2hook\mc2hook.h>
#include <age/vector/matrix34.h>
#include <age/vehicle/car.h>

class phArchetype;
class mcCar;

class vehEntity {
public:
	void* m_Vtable;
	phArchetype* m_Archetype;
	int dword_08;
	int dword_0c;
	Matrix34 m_Transform;
	mcCar m_Car;
};
