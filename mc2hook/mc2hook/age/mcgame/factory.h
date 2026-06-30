#pragma once
#include <mc2hook\mc2hook.h>

class vehEntity;

class mcPlayerFactory {
public:
	int dword_00;
	int dword_04;
	int dword_08;
	int dword_0c;

	// TODO: What's at 0x44?

public:
	vehEntity* MakeEntity();
	vehEntity* GetEntity() const;

	void MakeInstance();
	void MakeSim();
	void MakeInput();
	void MaybeMakeModel();
	void MakeDamage1();
	void MakeStuck();
	void MakeGyro();
	void MakeDriver();
	void MakeFeedback();
	void MaybeMakeCamera();
	void MaybeMakeWheelPtx();
	void MakeDamage2();
	void MaybeMakeAudio();
};
