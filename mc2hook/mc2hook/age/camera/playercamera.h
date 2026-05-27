#pragma once
#include <mc2hook\mc2hook.h>
#include <age/vector/matrix34.h>
#include <age/physics/phinst_old.h>
#include <age/vehicle/carsim.h>

class camTrackCS {

private:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	Matrix34 m_GlobalMatrix;
	Matrix34 m_SomeGlobalMatrixCopy;
	phInstOld* m_Instance;
	int dword_70;
	float m_BlendTime;
	float m_BlendGoal;
	float m_CurrentFOV;
	float dword_80;
	float dword_84;
	Vector3 m_TrackTo;
	int32_t m_ApproachOn;
	int32_t m_AppAppOn;
	float m_AppRot;
	float m_AppXRot;
	float m_AppYPos;
	float m_SomeMaxAppXZPos;
	float m_AppApp;
	float m_AppRotMin;
	float m_AppPosMin;
	float m_LookAbove;
	int dword_bc;
	float m_SomeSSDistMax;
	float m_SomeSSDistMin;
	float m_LookAt;
	Vector3 m_SomeLookAtPos;
	float dword_d8;
	float dword_dc;
	float dword_e0;
	float dword_e4;
	float dword_e8;
	float dword_ec;
	vehCarSim* m_CarSim;
	int m_ReverseOn;
	float m_JoltAmplitude;
	float m_JoltDuration;
	float m_Jolt;
	int dword_104;
	Vector3 m_Offset;
	int m_ColliderType;
	float m_ColliderHeight;
	float m_MinMaxOn;
	int m_TrackBreak;
	float m_MinAppXZPos;
	float m_MaxAppXZPos;
	float m_MinSpeed;
	float m_MaxSpeed;
	float m_AppInc;
	float m_AppDec;
	float m_MinHardSteer;
	float m_DriftDelay;
	float m_VertOffset;
	float m_FrontRate;
	float m_RearRate;
	float m_FlipDelay;
	int m_SteerOn;
	float m_SteerMin;
	float m_SteerAmt;
	float m_HillMin;
	float m_HillMax;
	float m_HillLerp;
	float m_RevDelay;
	float m_RevOnApp;
	float m_RevOffApp;
	int dword_178;
	float dword_17c;
	float m_SomeTimer;
	int dword_184;
	int dword_188;
	int dword_18c;
	int dword_190;
	int dword_194;
	int dword_198;
	int dword_19c;
	int dword_1a0;
	int dword_1a4;
	float m_Hill;
	int dword_1ac;
	int dword_1b0;
	int dword_1b4;
	int dword_1b8;
	int dword_1bc;
	int dword_1c0;
	float m_Steer;
	float m_Speed;
	int dword_1cc;
	bool field_1d0;
	float dword_1d4;
	float m_SomeDir_1d8;
	int dword_1dc;
	int dword_1e0;
	int dword_1e4;
	Vector3 m_SomePos_3;
	Vector3 m_SomePos_4;
	Vector3 m_SomePos_5;
	int dword_20c;
	int dword_210;
	float m_SSSpeedMin;
	float m_SSSpeedMax;
	float m_SSFOVMin;
	float m_SSFOVMax;
	float m_SSDistScale;
	float m_SSDistMin;
	float m_SSDistMax;
	float m_SSFilter;
	float m_SomeSpeed;

public:
	void UpdateSS(); // 0x46F140
	void Update();
	void UpdateTrack();

	void ApproachIt(); // camAppCS::ApproachIt()
	void UpdateApproach(); // camAppCS::UpdateApproach()
	void DApproach(float& a2, float a3, float a4, float a5, float& a6, float a7); // camAppCS::DApproach() // returns int?
	void UpdateMaxDist(); // camAppCS::UpdateMaxDist()
	void UpdateCar();
	void UpdateHill();
	void PreApproach();
};
