#include <mc2hook/mc2hook.h>
#include <age/memory/age_alloc_baseclass.h>
#include <veh_dyna/driver.h>

class mcCarSim;
class vehInput;

class mcCharacter : public vehDriver, public AGEAllocatedClass
{
public:
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
	int dword_1ac;
	int dword_1b0;
	int dword_1b4;
	int dword_1b8;
	int dword_1bc;
	int dword_1c0;
	int dword_1c4;
	int dword_1c8;
	int dword_1cc;
	int dword_1d0;
	int dword_1d4;
	int dword_1d8;
	int dword_1dc;
	int dword_1e0;
	int dword_1e4;
	int dword_1e8;
	int dword_1ec;
	int dword_1f0;
	int dword_1f4;
	int dword_1f8;
	int dword_1fc;
	int dword_200;
	int dword_204;
	int dword_208;
	int dword_20c;
	int dword_210;
	int dword_214;
	int dword_218;
	int dword_21c;
	int dword_220;
	int dword_224;
	int dword_228;
	int dword_22c;
	int dword_230;
	int dword_234;
	int dword_238;
	int dword_23c;
	int dword_240;
	float dword_244;
	int dword_248;
	float dword_24c;
	int dword_250;
	int dword_254;
	int dword_258;
	float dword_25c;
	int dword_260;
	int dword_264;
	int dword_268;
	int dword_26c;
	int dword_270;
	int dword_274;
	int dword_278;
	int dword_27c;
	int dword_280;
	int dword_284;
	int dword_288;

public:
	mcCharacter()  { hook::Thunk<0x4D8F80>::Call<void>(this); }
	~mcCharacter() { hook::Thunk<0x4D9090>::Call<void>(this); }

	void Init(mcCarSim* carSim, vehInput* input, const char* carName, const char* suffix, int a6, bool a7);
	void sub_4D8F30(int a2);
};

static_assert(sizeof(mcCharacter) == 0x28C, "mcCharacter size mismatch");
