#include "phlevel.h"
#include <age/physics/phcollider.h>
#include <age/physics/phsurface.h>
#include <age/physics/intersection.h>
#include <age/physics/bound.h>
#include <age/physics/archetype.h>

#include <age/core/output.h> //

declfield(phLevel::Instance)(0x6C8EB8);

phCollider* phLevel::GetCollider(int levelIndex)
{
	return hook::Thunk<0x580720>::Call<phCollider*>(this, levelIndex); // Call original
}

//phSurface* phLevel::GetMaterialFromIsect(phIntersection* isect)
//{
//	//Printf("Getting mat from isect\n");
//	
//	phSurface* mat = hook::StaticThunk<0x5808C0>::Call<phSurface*>(isect); // Call original
//	//Printf("mat = %p\n",mat); // Crashes?
//	return mat;
//}

//phSurface* phLevel::GetMaterialFromIsect(phIntersection* isect) // TODO: try int*?
//{
//	return hook::StaticThunk<0x5808C0>::Call<phSurface*>(isect); // Call original
//
//	phBound* bound = isect->m_Archetype->m_Bound;
//	phSurface* material = bound->sub_5A5550(isect->dword_24, HIWORD(isect->dword_28));
//	if (!material)
//		return phSurface::DefaultMaterial;
//	return material;
//}
