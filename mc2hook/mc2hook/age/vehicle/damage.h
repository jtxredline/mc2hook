#pragma once
#include <age/vector/vector3.h>
#include <age/vector/vector4.h>

class vehCarSim;

class vehDamage {
public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	int dword_0C;
	int dword_10;
	vehCarSim* m_CarSim;
	float m_MaxDamage;
	float m_MedDamage;
	float m_ImpactThreshold;
	float m_WipeoutThreshold;
	float m_WeightxferThreshold;
	float m_ExplodeThreshold;
	float m_JoltThreshold;
	float m_FatalBlow;
	float m_RegenerateRate;
	float m_WipeoutDelay;
	float m_DamageOutDelay;
	int dword_44;
	int dword_48;
	int dword_4C;
	int dword_50;
	int dword_54;
	float m_SomeCurrentDamage;
	float m_SomeLastDamage;
	float m_DamageOutTimer;
	float m_ExplodeImpulse;
	int dword_68;
	Vector3 m_LastImpactPos;
	Vector4 m_SparkStartColor;
	Vector4 m_SparkEndColor;
	Vector3 m_SparkVelVar;
	Vector3 m_RoofSparkVelVar;
	float m_SparkLifeTime;
	float m_SparkEmitMod;
	float m_SparkSize;
	int m_BottomOutCap;
	char field_C0;
	char field_C1;
	bool m_IsDamagedOut;
	char field_C3;
	char field_C4;

public:
	void sub_4CF500(int a2);
	bool sub_4CEC50();
	void sub_4CFCE0(float a2);
};
