#include "layermgr.h"

declfield(mcLayerMgr::Instance)(0x692E88);

void mcLayerMgr::BeginLoadLayer(int layer)
{
	hook::Thunk<0x4025D0>::Call<void>(this, layer); // Call original

	// if layer == movie
	m_LoadingMovie = true;
}

void mcLayerMgr::AfterLoadLayer(int layer)
{
	hook::Thunk<0x402680>::Call<void>(this, layer); // Call original

	// if layer == movie
	m_LoadingMovie = false;
}
