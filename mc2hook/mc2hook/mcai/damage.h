#pragma once
#include <mc2hook/mc2hook.h>
#include <age/memory/age_alloc_baseclass.h>
#include <mccar/cardamage.h>

class aiOpponent;

class aiDamage : public mcCarDamage, public AGEAllocatedClass
{
public:
	aiOpponent* m_Opponent;

	aiDamage(aiOpponent* opponent)  { hook::Thunk<0x4C4060>::Call<void>(this, opponent); }
	~aiDamage()                     { hook::Thunk<0x4C4090>::Call<void>(this); }
};

static_assert(sizeof(aiDamage) == 0xCC, "aiDamage size mismatch");
