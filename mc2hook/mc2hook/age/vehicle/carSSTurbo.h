#pragma once

class mcCarSSTurbo
{
public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	float m_TurboBoostExp;
	float m_TurboBoostAmount;
	float m_TurboRegenTime;
	float m_TurboUseTime;
	float m_TurboFalloffTime;
	float m_ChargedFalloffTime;
	int dword_24;
	float m_TurboFOV;
	float m_FOVOutTime;
	float m_FOVInTime;
	float dword_34;
	int dword_38;
	float dword_3c;
	float dword_40;
	float dword_44;
	float m_LoseChargeSpeed;
	float m_EnvelopeD1;
	float m_EnvelopeD2;
	float m_EnvelopeD3;
	float m_BlurOffTime;
	float m_BlurStartAmount;
	float dword_60;
	int dword_64;
	int dword_68;
	int dword_6c;
	int dword_70;
	int dword_74;
	float m_StayFullTime;
	float m_FlameTime;
	float m_FlameSize;
	int dword_84;
	int dword_88;
	int dword_8c;
	int dword_90;
	int dword_94;
	int dword_98;

public:
	void sub_4D4270();
	bool sub_4D4230();
	void sub_4D41F0();
};
