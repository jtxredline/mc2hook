#pragma once
#include <memory\age_alloc_baseclass.h>

class carAIInfo : public AGEAllocatedClass
{
public:
	void* m_Vtable;
	float m_TurnConst;
	float m_HBEarlyTime;
	float m_HBSteerMultiplier;
	float m_MaxBrakeDecel;
	float m_SteerScalar;
	float m_SteerGamma;
	float m_CarFrictionHandling;
	float m_FrictionMultiplier;
	float m_SlidingFrictionFactor;
public:
	carAIInfo()  { hook::Thunk<0x4DB940>::Call<void>(this); }
	~carAIInfo() { hook::Thunk<0x4DB850>::Call<void>(this); }
}; 
