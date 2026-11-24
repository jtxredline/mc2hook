#pragma once

class mcCarModelType
{
public:
	int m_Vtable;
	char m_Padding1[14196];
	void* m_Ptr1;
	void* m_Ptr2;
	void* m_Ptr3;
	void* m_Ptr4;
	char m_Padding2[488];
};

static_assert(sizeof(mcCarModelType) == 0x3970, "mcCarModelType size mismatch");
