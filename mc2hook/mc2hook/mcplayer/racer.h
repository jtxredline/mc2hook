#pragma once
#include <mc2hook/mc2hook.h>
#include <memory/age_alloc_baseclass.h>

class mcRacer : public AGEAllocatedClass
{
public:
	void* m_Vtable;
public:
	mcRacer()  { hook::Thunk<0x469E80>::Call<void>(this); }
	~mcRacer() { hook::Thunk<0x469EB0>::Call<void>(this); }
};
