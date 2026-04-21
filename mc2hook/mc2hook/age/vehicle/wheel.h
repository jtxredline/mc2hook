#pragma once
#include <mc2hook\mc2hook.h>
#include <age/vector/matrix34.h>
#include <age/physics/phinertia.h>

static constexpr float LRelaxCoef = 0.1f;
static constexpr float SlidingThreshold = 0.5f;

class vehCarSim;
class phInertialCS;

class vehWheel {
public:
	void* m_Vtable;
	vehCarSim* m_CarSim;
	phInertialCS* m_ICS;
	Matrix34 m_GlobalMatrixWheelCenter;
	float m_TireDispLimitLat;
	float m_TireDispLimitLong;
	float m_TireDispCoefLat;
	float m_TireDispCoefLong;
	float m_TireDragCoefLat;
	float m_TireDragCoefLong;
	float m_SteeringLimit;
	float m_CamberLimit;
	float m_WobbleLimit;
	float m_AxleLimit;
	float m_BrakeCoef;
	float m_HandbrakeCoef;
	float m_SteeringOffset;
	float m_VerticalSurfaceThreshold;
	float m_SuspensionLimit;
	float m_SuspensionExtent;
	float m_SuspensionClamp;
	float m_SuspensionFactor;
	float m_SuspensionDampCoef;
	int m_MaybeHasIntersection;
	Vector3 m_RayStartPos;
	Vector3 m_RayEndPos;
	void* m_SomeUnkPtr;
	Vector3 m_LastContactPosition;
	int dword_b4;
	int dword_b8;
	int dword_bc;
	int dword_c0;
	int dword_c4;
	int dword_c8;
	int dword_cc;
	bool m_HasIntersection;
	char m_HasIntersectionPadding[3];
	float m_WheelVelLat;
	float m_WheelVelLong;
	float m_ForceOrVelLat; // Maybe VelLatRelative
	float m_SlipVelLong;
	Matrix34 m_ContactMatrix;
	Vector3 m_LocalOffset;
	float m_SteeringPivotOffset;
	float m_Radius;
	float m_Width;
	float m_NormalLoad;
	int dword_130;
	float m_SurfaceDrag;
	float m_SurfaceFriction;
	float m_SurfaceDepth;
	float m_SurfaceHeight;
	float m_SurfaceWidth;
	int dword_148;
	float m_AxleValue;
	float m_BrakeValue;
	float m_BrakeTorque;
	float m_HandbrakeTorque;
	float m_SteeringAngle;
	float m_SuspensionValue;
	float m_SomeTireLoad;
	float m_SuspensionVelocity;
	float m_SomeNormalLoad;
	float m_SomeSuspensionForceOrLoad;
	float field_174;
	int dword_178;
	int dword_17c;
	int dword_180;
	float m_SlidingStrength;
	float m_FrictionHandling;
	bool m_UsePivotOffset;
	bool m_IsSliding;
	bool m_OnGround;
	bool m_BottomedOut;
	float m_TireDispLat;
	float m_TireDispLong;
	float m_TireForceLat;
	float m_TireForceLong;
	float m_WheelDriveTorque;
	float m_SomeAngularVelocity;
	float m_SlipPercentLat;
	float m_SlipPercentLong;
	float m_StiffnessLong;
	float m_DampingLong;
	float m_OptimumSlipPercent;
	float m_StaticFric;
	float m_SlidingFric;
	float m_StiffnessLat;
	float m_DampingLat;
	float m_InvOSPSquared;
	void* m_PhysMtl;

public:
	void Update();
	void UpdateSuspensionRay();
	void CalcDispAndDamp(float* disp, float dispTarget, float step, float limit, float* outStep, bool* outFree);
	void ComputeSlipPercent(float* slipPercent, float slipVelocity, float referenceVelocity);
	float ComputeFriction(float slipPercent, float* outSlipRatio) const;
	float GetVisualDispVert() const;
	
	void UpdateComp();
	void UpdateVisuals();
};
//ASSERT_SIZEOF(vehWheel, 0x1D4);
