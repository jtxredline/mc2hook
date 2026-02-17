#include <age/math/math.h>
#include <math.h> // cos?
#include <stdlib.h> // for frand

namespace math
{
    bool Approach(float& value, float target, float speed, float dt)
    {
        if (value > target)
        {
            value -= speed * dt;
            if (value < target) value = target;
        }
        else if (value < target)
        {
            value += speed * dt;
            if (value > target) value = target;
        }

        return value == target;
    }

    float InverseLerp(float value, float min, float max)
    {
        if (value <= min)
            return 0.0;
        if (value < max)
            return (value - min) / (max - min);
        return 1.0;
    }

    float EaseIn(float value)
    {
        if (value <= 0.0f) return 0.0;
        if (value >= 1.0f) return 1.0;

        return 1.0 - cos(value * HALF_PI);
    }

    float DistanceSq(Vector3 from, Vector3 to)
    {
        float dx, dy, dz;
        dx = from.X - to.X;
        dy = from.Y - to.Y;
        dz = from.Z - to.Z;
        return dx * dx + dy * dy + dz * dz;
    }

    float frand()
    {
        return float(rand()) / float(RAND_MAX);
    }

    float Sign(float value)
    {
        if (value > 0.0f) return 1.0f;
        if (value < 0.0f) return -1.0f;
        return 0.0f;
    }

    int RandomInt(int min, int max)
    {
        static unsigned int seed = 0x12345678;
        seed = seed * 1103515245 + 12345;
        return min + (seed % (max - min + 1));
    }
}
