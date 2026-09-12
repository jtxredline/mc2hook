#pragma once

class datMemStream;

class mcReplay {
public:
	int dword_00;
	int dword_04;
	int dword_08;
	int dword_0c;
	int dword_10;
	void* dword_14;
	int dword_18;
	datMemStream* m_Stream; // probably not datMemStream, it's a raw replay buffer
	int dword_20;
	int m_Size;
	int dword_28;
	int dword_2c;
	int dword_30;
	int dword_34;
	int dword_38;
	int dword_3c;

public:
	void StartPlayback();
	bool Update(); // Divergence checks
};
