#pragma once

class mcCarSim;

class vehDriver
{
public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	int dword_0c;
	int dword_10;
	float dword_14;
	int dword_18;
	int dword_1c;
	int dword_20;
	float dword_24;
	int dword_28;
	int dword_2c;
	int dword_30;
	float dword_34;
	int dword_38;
	int dword_3c;
	int dword_40;
	float dword_44;
	int dword_48;
	int dword_4c;
	int dword_50;
	float dword_54;
	int dword_58;
	int dword_5c;
	int dword_60;
	float dword_64;
	int dword_68;
	int dword_6c;
	int dword_70;
	int dword_74;
	int dword_78;
	int dword_7c;
	int dword_80;
	int dword_84;
	int dword_88;
	int dword_8c;
	int dword_90;
	int dword_94;
	int dword_98;
	float m_Filter;
	float m_LiftVel;
	float m_LiftAcc;
	float m_LiftMin;
	float m_LiftMax;
	float m_SlideVel;
	float m_SlideAcc;
	float m_SlideMin;
	float m_SlideMax;
	float m_LeanVel;
	float m_LeanAcc;
	float m_LeanMin;
	float m_LeanMax;
	float dword_d0;
	float dword_d4;

public:
	void Init(mcCarSim* carSim, const char* carName);
};
