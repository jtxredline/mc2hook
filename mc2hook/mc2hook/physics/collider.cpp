#include "collider.h"
#include <phinertia/inertia.h>

declfield(phCollider::SomeVtable)(0x6508DC);

void phCollider::ApplyLeanImpulse(float impulse)
{
    if (!m_ICS) return;

    // Scale impulse by angular inertia (Z = roll axis)
    float torqueMag = impulse * m_ICS->m_AngInertia.Z;

    m_ICS->m_Torque.X += torqueMag * m_ICS->m_WorldTransform.m20;
    m_ICS->m_Torque.Y += torqueMag * m_ICS->m_WorldTransform.m21;
    m_ICS->m_Torque.Z += torqueMag * m_ICS->m_WorldTransform.m22;

    //// Apply torque along the collider's forward (Z) axis in world space
    //const Matrix34& transform = m_ICS->m_WorldTransform;

    //m_ICS->m_Torque += transform.GetRow(2) * torqueMag;
}

void phCollider::Init(vehEntity* entity, phInertialCS* ics, int a3)
{
    return hook::Thunk<0x591C60>::Call<void>(this, entity, ics, a3); // Call original
}
