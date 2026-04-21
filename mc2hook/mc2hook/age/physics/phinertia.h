#pragma once
#include <age/vector/matrix34.h>

class phInertialCS {
public:
	void* m_Vtable;
	float m_Mass;
	float m_InvMass;
	Vector3 m_AngInertia;
	Vector3 m_InvAngInertia;
	float m_MaxVelocity;
	Vector3 m_MaxAngVelocity;
	Vector3 m_Momentum;
	Vector3 m_AngularMomentum;
	Matrix34 m_WorldTransform;
	Vector3 m_WorldVelocity;
	Vector3 m_AngularVelocity;
	Vector3 m_Force;
	Vector3 m_Torque;
	Vector3 m_SomeOversampleForce;
	Vector3 m_OversampleTorque;
	bool m_NeedsOversampling;
	Vector3 m_Impulse;
	Vector3 m_AngularImpulse;
	Vector3 m_Push;
	Vector3 m_Turn;
	Vector3 m_AppliedPush;
	Vector3 m_LastPush;
	int dword_110;
	int dword_114;
	int dword_118;
	int dword_11c;
	int dword_120;
	int dword_124;
	int dword_128;
	int dword_12c;
	int dword_130;
	int dword_134;
	int dword_138;
	int dword_13c;
	int dword_140;
	int dword_144;
	int dword_148;
	int dword_14c;
	int dword_150;
	int dword_154;
	int dword_158;
	int dword_15c;
	int dword_160;
	int dword_164;
	int dword_168;
	int dword_16c;
	int dword_170;
	int dword_174;
	int dword_178;
	int dword_17c;
	int dword_180;
	int dword_184;
	int dword_188;
	int dword_18c;
	int dword_190;
	int dword_194;
	int dword_198;
	int dword_19c;
	int dword_1a0;
	int dword_1a4;
	int dword_1a8;

public:
	void ApplyContactForce(Vector3* someForce,Vector3* somePos, Matrix34* a4, Vector3* a5);
	void AddForce(const Vector3& f); // Vector3* a3);
	void AccumulateForce(const Vector3& force);
	void ComputeForce(float impulseScale, Vector3& out) const;
	void ApplyForwardTorque(float amount);
	void ApplyUpTorque(float amount);
	void ApplyLateralTorque(float amount);
};
