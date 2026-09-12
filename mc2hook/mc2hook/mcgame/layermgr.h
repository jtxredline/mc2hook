#pragma once
#include <mc2hook\mc2hook.h>

class mcLayerMgr
{
public:
	static hook::Type<mcLayerMgr*> Instance;

public:
	int dword_00;
	int dword_04;
	int dword_08;
	int dword_0c;
	int dword_10;
	int dword_14;
	int dword_18;
	int dword_1c;
	int dword_20;
	int dword_24;
	int dword_28;
	int dword_2c;
	int dword_30;
	int dword_34;
	int dword_38;
	int dword_3c;
	int dword_40;
	int dword_44;
	int dword_48;
	int dword_4c;
	int dword_50;
	int dword_54;
	int dword_58;
	int dword_5c;
	int dword_60;
	int dword_64;
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
	int dword_9c;
	int dword_a0;
	int dword_a4;
	int dword_a8;
	int dword_ac;
	int dword_b0;
	int dword_b4;
	int dword_b8;
	int dword_bc;
	int dword_c0;
	int dword_c4;
	int dword_c8;
	int dword_cc;
	int dword_d0;
	int dword_d4;
	int dword_d8;
	int dword_dc;
	int dword_e0;
	int dword_e4;
	int dword_e8;
	int dword_ec;
	int dword_f0;
	int dword_f4;
	int dword_f8;
	int dword_fc;
	int dword_100;
	int dword_104;
	int dword_108;
	int dword_10c;
	int dword_110;
	int dword_114;
	int dword_118;
	int dword_11c;
	int dword_120;
	int dword_124;
	int dword_128;

public:
	bool m_LoadingMovie; //

public:
	void BeginLoadLayer(int layer);
	void AfterLoadLayer(int layer);
};
