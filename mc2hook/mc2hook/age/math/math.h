#pragma once
#include <age/vector/vector3.h>

namespace math
{
	constexpr float HALF_PI = 1.57079632679f;
	constexpr float PI = 3.141592653f;
	constexpr float PI2 = 6.283185306f;

	constexpr float RPS_TO_RPM = 9.549296f; // radians/sec -> rotations/min
	constexpr float RPM_TO_RPS = 0.1047198f; // rotations/min -> radians/sec
	constexpr float NMS_TO_HP = 0.001340483f; // Newton*meter/sec -> horsepower
	constexpr float DEG_TO_RAD = 0.01745329f;
	constexpr float RAD_TO_DEG = 57.2957795f;
	constexpr float FIBONACCI_NUMBER = 1.618034f;


	bool Approach(float& value, float target, float speed, float dt); // Moves value towards a target value at a certain speed, returns TRUE if the target has been reached.
	float InverseLerp(float value, float min, float max); // Maps value into a normalized range 0–1, based on mix/max thresholds.
	float EaseIn(float value); // Applies a cosine-based ease-in curve to a normalized input.
	float DistanceSq(Vector3 from, Vector3 to); // Distance/length squared.
	float frand();
	float Sign(float value);
	int RandomInt(int min, int max);
}
