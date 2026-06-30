#include "vehinput.h"
#include <mc2hook\mc2hook.h>
#include <age/mcnetwork/netmanager.h>
#include <age/state/gamestate.h>
#include <age/state/racestate.h>
#include <age/vehicle/entity.h>
#include <age/vehicle/carsim.h>
#include <age/vehicle/carmodel.h>
#include <age/vehicle/caraudio.h>
#include <age/vehicle/damage.h>
#include <age/vehicle/carSSTurbo.h>
#include <age/vehicle/nitro.h>
#include <age/vehicle/transmission.h>
#include <age/math/math.h>
#include <age/data/replay.h>
#include <age/data/timemgr.h>
#include <age/mcaudio/mcaudiomgr.h>
#include <age/input/joystick.h>
#include <age/input/device.h>
#include <age/physics/phcollider.h>
#include <age/physics/phinertia.h>
#include <age/vector/vector3.h>
#include <age/vector/matrix34.h>

#include <age/core/output.h>

// Kinematic steering state
static bool kinematicEnabled = false;
static float steeringRate = 8.0f;
static float steeringLock = 1.0f;
static float speedEnd = 100.0f;
static float sCurveSharpness = 0.1f;
static float maxSteeringFactor = 1.0f;
static float minSteeringFactor = 0.89f;
static float gyroGain = 1.0f;
static float slipAngleGain = 2.0f;
static float steerRange = 0.0f;
static float speedEndHalf = 0.0f;
static float invSpeedEnd = 0.0f;
static float steerValue = 0.0f;

static void LoadKinematicConfig()
{
    steeringRate = HookConfig::GetFloat("KinematicSteer", "SteeringRate", 8.0f);
    steeringLock = HookConfig::GetFloat("KinematicSteer", "SteeringLock", 1.0f);
    speedEnd = HookConfig::GetFloat("KinematicSteer", "SpeedEnd", 100.0f);
    sCurveSharpness = HookConfig::GetFloat("KinematicSteer", "SCurveSharpness", 0.1f);
    maxSteeringFactor = HookConfig::GetFloat("KinematicSteer", "MaxSteeringFactor", 1.0f);
    minSteeringFactor = HookConfig::GetFloat("KinematicSteer", "MinSteeringFactor", 0.89f);
    gyroGain = HookConfig::GetFloat("KinematicSteer", "GyroGain", 1.0f);
    slipAngleGain = HookConfig::GetFloat("KinematicSteer", "SlipAngleGain", 2.0f);
    steerRange = maxSteeringFactor - minSteeringFactor;
    speedEndHalf = speedEnd * 0.5f;
    invSpeedEnd = 1.0f / speedEnd;
}

static float ComputeNormalizedSpeed(float velocityMagnitude)
{
    return fminf(velocityMagnitude * invSpeedEnd, 1.0f);
}

static float ComputeGyroCorrection(float angularVelocityY, float deltaTime)
{
    return (-angularVelocityY * deltaTime) * gyroGain;
}

static float ComputeSlipCorrection(float lateralVelocity, float longitudinalVelocity, float deltaTime, float normalizedSpeed)
{
    float slipAngle = (fabsf(lateralVelocity) > 0.1f) ? atan2f(lateralVelocity, -longitudinalVelocity) : 0.0f;
    return -slipAngle * slipAngleGain * deltaTime * normalizedSpeed;
}

static float ComputeSpeedFactor(float normalizedSpeed)
{
    float expArg = -sCurveSharpness * (normalizedSpeed * speedEnd - speedEndHalf);
    return fmaxf(minSteeringFactor, fminf(maxSteeringFactor - steerRange / (1.0f + expf(expArg)), maxSteeringFactor));
}

static float ClampSteering(float value)
{
    return fminf(fmaxf(value, -steeringLock), steeringLock);
}

static Vector3 ComputeAxleVelocity(const Vector3& worldVelocity, const Vector3& angularVelocity, const Matrix34& worldTransform, float axleOffsetX)
{
    Vector3 xAxis = worldTransform.GetRow(0);
    Vector3 axleOffsetWorld = xAxis * axleOffsetX;
    Vector3 rotVel = Vector3::Cross(angularVelocity, axleOffsetWorld);
    return worldVelocity + rotVel;
}

void vehInput::Update()
{
    // hook::Thunk<0x46B330>::Call<void>(this); // Call original

    int localPlayerID = 0;
    if (mcNetManager::IsNetworkMode) localPlayerID = mcNetManager::LocalPlayerID;

    if (localPlayerID != static_cast<int>(m_PlayerID))
        goto NETWORK;

    // Mirror mode check
    static bool mirrorMode = HookConfig::GetBool("Graphics", "MirrorMode", false);

    // Update device state
    m_Device->sub_467DF0();

    int& flags = this->m_CurrentGearFlags;

    // Analog gas/brake
    float analogGasBrake = 0.0f;

    if (!mcGameState::Instance->m_IsPausedLocally)
    {
        bool digitalMode = m_Device->sub_467FA0();

        if (digitalMode) analogGasBrake = -m_Device->GetInput(8);
        else analogGasBrake = -m_Device->GetInput(6) - m_Device->GetInput(7) + m_Device->GetInput(8); // Likely not steer but gas/brake axis

        analogGasBrake -= m_Device->GetInput(0xA);
        analogGasBrake += m_Device->GetInput(9);

        analogGasBrake = math::Clamp(analogGasBrake, -1.0f, 1.0f);
    }

    // Gas / brake
    m_Brake = 0.0f;
    m_GasBrake = 0.0f;
    m_Drivable = 0;

    sub_46A7A0(analogGasBrake, &m_GasBrake, &m_Brake, &m_Drivable);

    // Transmission flags
    vehTransmission* transmission = m_CarSim->m_Transmission;
    bool isAuto = transmission->m_Mode == TransmissionMode::Auto;

    flags = (flags & 0xFFFF0000) | (transmission->m_CurrentGear & 0xFFFF);

    // Handbrake
    float hb = (!mcGameState::Instance->m_IsPausedLocally && m_Device->GetInput(0xD) != 0.0f) ? 1.0f : 0.0f;

    if (dword_70 <= 0.0f || hb <= m_Handbrake) m_Handbrake = hb;
    else math::Approach(m_Handbrake, hb, dword_70, datTimeManager::Seconds);

    if (m_Drivable < 0 && m_Handbrake > 0.0f) m_GasBrake = 0.0f;

    // Controller detection
    bool controllerActive = false;
    if ((int)ioJoystick::MaybeActiveJoystick > -1)
    {
        controllerActive = hook::Thunk<0x604CE0>::Call<bool>((int)(&ioJoystick::sm_Sticks + 0x25 * ioJoystick::MaybeActiveJoystick));
    }

    // Steering
    float steerAxis = m_Device->GetInput(0);
    float left = m_Device->GetInput(1);
    float right = m_Device->GetInput(2);

    // Mirror mode adjustments
    if (mirrorMode)
    {
        // Mirror analog steering
        steerAxis = -steerAxis;

        // Swap left-right digital inputs
        float tmp = left;
        left = right;
        right = tmp;
    }

    m_Steer = steerAxis - left + right;
    m_Steer = math::Clamp(m_Steer, -1.0f, 1.0f);

    // Controller curve
    if (controllerActive)
    {
        float sign = (m_Steer < 0.0f) ? -1.0f : 1.0f;
        float absSteer = std::abs(m_Steer);

        float curved = (absSteer + sqrt(absSteer)) * 0.5f;
        m_Steer = curved * sign;
    }

    int airControlFlag = 0; // ?

    // Keyboard / fallback steering

    if (ioJoystick::MaybeActiveJoystick == -1
        || !ioJoystick::dword_85FC90[0x25 * ioJoystick::MaybeActiveJoystick]
        || controllerActive)
    {
        if (m_Device->GetInput(0xE) != 0.0f)
        {
            if (m_Steer > 0.75f && m_InAirSteer <= 0.75f)
                airControlFlag = 1;

            if (m_Steer < -0.75f && m_InAirSteer >= -0.75f)
                airControlFlag = 1;
        }

        goto LABEL_61;
    }

    // Analog steering smoothing
    float speedFactor = sub_46AA90(m_CarSim->m_Speed);
    float steer = m_Steer;

    if (dword_74 > 0.0f)
    {
        if ((m_Steer > 0.95f && speedFactor * 0.9f < steer) ||
            (m_Steer < -0.95f && speedFactor * -0.9f > steer))
        {
            math::Approach(steer, m_Steer, dword_74, datTimeManager::Seconds);
        }
        else
        {
            float target = speedFactor * m_Steer;

            if (m_AnalogSteerSpeed <= 0.0f)
                steer = target;
            else
                math::Approach(steer, target, m_AnalogSteerSpeed, datTimeManager::Seconds);
        }
    }
    else
    {
        steer = speedFactor * m_Steer;
    }

LABEL_46:
    if (m_Device->GetInput(0xE) != 0.0f)
    {
        if (m_Steer > 0.75f && m_InAirSteer <= 0.75f)
            airControlFlag = 1;

        if (m_Steer < -0.75f && m_InAirSteer >= -0.75f)
            airControlFlag = 1;
    }

    m_Steer = math::Clamp(steer, -1.0f, 1.0f);

LABEL_61:
    m_InAirSteer = m_Steer;

    // Flag updates
    if (airControlFlag)
        flags |= (0x80u << 16);
    else
        flags &= ~(0x80u << 16);

    if (m_Device->GetInput(0xE) != 0.0f)
        flags |= (1u << 16);
    else
        flags &= ~(1u << 16);

    sub_46A760(0, dword_44);

    // Input 0x16
    if (mcGameState::Instance->m_IsPausedLocally || m_Device->GetInput(0x16) == 0.0f)
    {
        // Clear bits except mask 0xED
        flags = (flags & ~(0xFFu << 16)) |
            (((flags >> 16) & 0xEDu) << 16);
    }
    else
    {
        vehDamage* damage = m_Entity->m_Car.m_Damage;
        if (damage && damage->sub_4CEC50())
        {
            // Set bit 0x10
            flags = (flags & 0x0000FFFF) | ((((flags >> 16) & 0xFFEDu) | 0x10u) << 16);
        }
        else
        {
            // Set bit 0x02
            flags = (flags & 0x0000FFFF) | ((((flags >> 16) & 0xFFEDu) | 2u) << 16);
        }
    }

    if (mcGameState::Instance->m_IsPausedLocally || m_Device->GetInput(0x10) == 0.0f) flags &= ~(0x20u << 16);
    else flags |= (0x20u << 16);

    // Input 0xF - SSTurbo / Nitro
    if (mcGameState::Instance->m_IsPausedLocally || m_Device->GetInput(0xF) == 0.0f)
    {
        // clear turbo/nitro bits (mask 0xF3)
        flags = (flags & ~(0xFFu << 16)) | (((flags >> 16) & 0xF3u) << 16);
    }
    else
    {
        mcCarSSTurbo* ssturbo = m_Entity->m_Car.m_CarSim->m_SSTurbo;
        vehNitro* nitro = m_Entity->m_Car.m_CarSim->m_Nitro;

        if (ssturbo && ssturbo->sub_4D4230())
        {
            // SSTurbo active - bit 0x04
            flags = (flags & 0x0000FFFF) | ((((flags >> 16) & 0xFFF3u) | 4u) << 16);
        }
        else if (nitro && nitro->sub_46A350() && !LOBYTE(ssturbo->dword_70))
        {
            // Nitro active (only if SST not blocking) - bit 0x08
            flags = (flags & 0x0000FFFF) | ((((flags >> 16) & 0xFFF3u) | 8u) << 16);
        }
    }

    // Input 0x17
    if (mcGameState::Instance->m_IsPausedLocally || m_Device->GetInput(0x17) == 0.0f) flags &= ~(0x40u << 16);
    else flags |= (0x40u << 16);

    // Map toggle
    //
    //if (!mcGameState::Instance->m_IsPausedLocally
    //    && m_Device->sub_467E90(0x13)//ioDevice_467E90(&this->device->unk0, 0x13)
    //    && mcRaceState::Instance->m_CurrentState < 6)
    //{
    //    v35 = *(_BYTE**)(*(_DWORD*)(sub_4690F0(SHIWORD(this->player_id_flags)) + 0x50) + 0x14);
    //    if (v35[0x28])
    //        (*(void(__thiscall**)(_BYTE*, _DWORD))(*(_DWORD*)v35 + 0x34))(v35, 0);
    //    else
    //        (*(void(__thiscall**)(_BYTE*, int))(*(_DWORD*)v35 + 0x34))(v35, 1);
    //}

    if (!mcGameState::Instance->m_IsPausedLocally
        && m_Device->sub_467E90(0x13)
        && mcRaceState::Instance->m_CurrentState < 6)
    {
        int playerId = m_PlayerID;//SHIWORD(this->player_id_flags);

        void* playerMgr = hook::StaticThunk<0x4690F0>::Call<void*>(playerId); // GetPlayerManager_4690F0(playerId);
        if (playerMgr)
        {
            void* ptr50 = *(void**)((char*)playerMgr + 0x50);
            if (ptr50)
            {
                uint8_t* mapCtrl = *(uint8_t**)((char*)ptr50 + 0x14);
                if (mapCtrl)
                {
                    // Offset 0x28 = "is map open"
                    bool isOpen = mapCtrl[0x28] != 0;

                    // Virtual function at vtable + 0x34
                    void* vtable = *(void**)mapCtrl;
                    using Fn = void(__thiscall*)(void*, int);
                    Fn toggle = *(Fn*)((char*)vtable + 0x34);

                    toggle(mapCtrl, isOpen ? 0 : 1);
                }
            }
        }
    }
    // Map toggle end

    //
    if (!mcGameState::Instance->m_IsPausedLocally && m_Device->sub_467E90(0x14))
    {
        dword_40 = (dword_40 + 1 >= 0 ? 0 : 4) + (dword_40 + 1) % 4;
    }

    if (!mcGameState::Instance->m_IsPausedLocally && m_Device->sub_467E90(0x15))
    {
        dword_40 = (dword_40 - 1 >= 0 ? 0 : 4) + (dword_40 - 1) % 4;
    }

    if (!mcGameState::Instance->m_IsPausedLocally && m_Device->sub_467E90(0x1C))
    {
        mcAudioManager::Instance->sub_52B3D0();
    }

    if (!mcGameState::Instance->m_IsPausedLocally && m_Device->sub_467E90(0x1B))
    {
        mcAudioManager::Instance->sub_52B3C0();
    }

    //
    if ((flags & 0x810000) != 0)
    {
        float steerRate = m_Device->GetInput(5);

        if (m_Device->GetInput(3) != 0.0f) steerRate += 1.0f;
        if (m_Device->GetInput(4) != 0.0f) steerRate -= 1.0f;

        if (m_AnalogSteerSpeed <= 0.0f || (flags & 0x800000))
            m_SteerRate = steerRate;
        else
            math::Approach(m_SteerRate, steerRate, m_AnalogSteerSpeed, datTimeManager::Seconds);

        if (right != 0.0f) steerAxis += 1.0f;
        if (left != 0.0f) steerAxis -= 1.0f;

        if (m_AnalogSteerSpeed <= 0.0f || (flags & 0x800000)) m_2WheelSteer = steerAxis;

        else math::Approach(m_2WheelSteer, steerAxis, m_AnalogSteerSpeed, datTimeManager::Seconds);
    }
    else
    {
        this->m_SteerRate = 0;
        this->m_2WheelSteer = 0.0;
    }

    // Manual transmission input
    if (!isAuto)
    {
        if (m_Device->sub_467E90(0xB))
        {
            flags |= (1u << 24); // Upshift flag
        }
        else if (m_Device->sub_467E90(0xC))
        {
            flags |= (2u << 24); // Downshift flag
        }
        else
        {
            flags = (flags & ~(0xFFu << 24)) | (((flags >> 24) & 0xFCu) << 24);
        }

        if ((flags & 0x1000000) != 0) m_CarSim->m_Transmission->Upshift();
        else if ((flags & 0x2000000) != 0) m_CarSim->m_Transmission->Downshift();
    }

    // Sync gear
    flags = (flags & 0xFFFF0000) | (m_CarSim->m_Transmission->m_CurrentGear & 0xFFFF);

    // Kinematic steering
    {
        static bool initialized = false;
        if (!initialized)
        {
            kinematicEnabled = HookConfig::GetBool("KinematicSteer", "KinematicSteerEnable", false);
            if (kinematicEnabled) LoadKinematicConfig();
            initialized = true;
        }

        if (kinematicEnabled && m_CarSim && m_CarSim->m_Collider && m_CarSim->m_Collider->m_ICS)
        {
            float lastRequestedSteering = m_Steer;
            float deltaTime = datTimeManager::GetSeconds();
            phInertialCS* ics = m_CarSim->m_Collider->m_ICS;

            Vector3 worldVel = ics->m_WorldVelocity;
            Matrix34 worldTrans = ics->m_WorldTransform;
            Vector3 angVel = ics->m_AngularVelocity;

            float frontAxleX = m_CarSim->m_Wheels[0]->m_LocalOffset.X;
            Vector3 axleVel = ComputeAxleVelocity(worldVel, angVel, worldTrans, frontAxleX);

            float lateralVel = axleVel.Dot(worldTrans.GetRow(0));
            float longitudinalVel = axleVel.Dot(worldTrans.GetRow(2));

            float normalizedSpeed = ComputeNormalizedSpeed(worldVel.Mag());
            float gyroCorr = ComputeGyroCorrection(angVel.Y, deltaTime);
            float slipCorr = ComputeSlipCorrection(lateralVel, longitudinalVel, deltaTime, normalizedSpeed);

            float directionSign = (longitudinalVel >= 0.0f) ? -1.0f : 1.0f;
            float correctionDelta = slipCorr + gyroCorr * directionSign;
            float driverDelta = lastRequestedSteering * steeringRate * deltaTime;

            float speedFactor = ComputeSpeedFactor(normalizedSpeed);
            float fpsCorrectionPower = deltaTime * 144.0f;
            float normalizedDampening = powf(speedFactor, fpsCorrectionPower);

            steerValue = ClampSteering((steerValue + driverDelta - correctionDelta) * normalizedDampening);
            m_Steer = steerValue;
        }
    }

    // Apply to car
    if (!mcNetManager::IsNetworkMode)
    {
        m_Entity->m_Car.m_CarSim->SetDrivable(m_Drivable);
        m_Entity->m_Car.m_CarSim->m_Steer = m_Steer;
        m_Entity->m_Car.m_CarSim->m_Throttle = m_GasBrake;
        m_Entity->m_Car.m_CarSim->m_Brake = m_Brake;
        m_Entity->m_Car.m_CarSim->m_Handbrake = m_Handbrake;

        if (!isAuto) m_Entity->m_Car.m_CarSim->m_Transmission->SetCurrentGear((int16_t)(flags & 0xFFFF));

        m_Entity->m_Car.m_Model->sub_4C4BC0((flags & 0x20000) != 0);

        if (mcRaceState::Instance->m_CurrentState < 6 && !mcGameState::Instance->m_IsPaused)
            m_Entity->m_Car.m_Audio->sub_4D6800((flags & 0x400000) != 0);

        // Boost triggers
        if (flags & 0x40000)
        {
            if (mcCarSSTurbo* ssturbo = m_Entity->m_Car.m_CarSim->m_SSTurbo)
                ssturbo->sub_4D4270();
        }

        if (flags & 0x80000)
        {
            if (vehNitro* nitro = m_Entity->m_Car.m_CarSim->m_Nitro)
                nitro->sub_4D1F80();
        }

        // Damage feedback
        uint32_t hi = flags >> 16;

        if (hi & 0x10)
        {
            if (vehDamage* damage = m_Entity->m_Car.m_Damage)
            {
                damage->sub_4CF500((hi >> 5) & 0xFFFFFF01);
            }
        }
    }

NETWORK:
    UpdateFFB();
    UpdateNetworkInput();
}

void vehInput::UpdateFFB()
{
    hook::Thunk<0x46A450>::Call<void>(this);
}

void vehInput::UpdateNetworkInput()
{
    // hook::Thunk<0x46AE60>::Call<void>(this); // Call original

    if (!mcNetManager::IsNetworkMode) return;
    
    bool isLocalPlayer = mcNetManager::LocalPlayerID == static_cast<int>(m_PlayerID);

    if (isLocalPlayer)
    {
        // net_84
        PackUnsignedFloat(m_GasBrake, &net_84[2]);
        PackUnsignedFloat(m_Brake, &net_84[3]);

        // net_88
        PackUnsignedFloat(m_Handbrake, &net_88[0]);
        PackSignedFloat(m_Steer, reinterpret_cast<int8_t*>(&net_88[1]));

        // Gear flags split across buffers
        net_88[2] = (uint8_t)(m_CurrentGearFlags & 0xFF);
        *(uint16_t*)&net_84[0] = (uint16_t)(m_CurrentGearFlags >> 16);

        PackSignedFloat(m_SteerRate, reinterpret_cast<int8_t*>(&net_88[3]));

        // net_8C
        PackSignedFloat(m_2WheelSteer, reinterpret_cast<int8_t*>(&net_8C[0]));

        net_8C[1] = (uint8_t)m_Drivable;
        net_8C[2] = 1; // Mark packet as valid
    }

    // Unpack (network -> local sim)
    if (!net_8C[2])
        return;

    m_GasBrake = net_84[2] * 0.0039215689f; // (1.0f / 255.0f);
    m_Brake = net_84[3] * 0.0039215689f;
    m_Handbrake = net_88[0] * 0.0039215689f;

    m_Steer = (int8_t)net_88[1] * 0.0078740157; // (1.0f / 127.0f);
    m_SteerRate = (int8_t)net_88[3] * 0.0078740157;
    m_2WheelSteer = (int8_t)net_8C[0] * 0.0078740157;

    // Reconstruct gear flags
    uint16_t high = *(uint16_t*)&net_84[0];
    uint8_t low = net_88[2];

    m_CurrentGearFlags = (high << 16) | low;

    // Drivable flag
    m_Drivable = (int8_t)net_8C[1];

    // Apply to simulation
    m_Entity->m_Car.m_CarSim->SetDrivable(m_Drivable);

    m_CarSim->m_Throttle = m_GasBrake;
    m_CarSim->m_Brake = m_Brake;
    m_CarSim->m_Handbrake = m_Handbrake;
    m_CarSim->m_Steer = m_Steer;

    //if (m_CurrentGearFlags & 0x1000000) m_CarSim->m_Transmission->Upshift();
    //else if (m_CurrentGearFlags & 0x2000000) m_CarSim->m_Transmission->Downshift();

    // Fix for double shift and no neutral/reverse in manual in MP
    if (m_CarSim->m_Transmission->m_Mode != TransmissionMode::Auto)
    {
        int16_t gear = (int16_t)(m_CurrentGearFlags & 0xFFFF);
        m_CarSim->m_Transmission->SetCurrentGear(gear);
    }
    
    // Headlights
    m_Entity->m_Car.m_Model->sub_4C4BC0((m_CurrentGearFlags & 0x20000) != 0);

    // Horn
    if (mcRaceState::Instance->m_CurrentState < 6 && !mcGameState::Instance->m_IsPaused)
    {
        m_Entity->m_Car.m_Audio->sub_4D6800((m_CurrentGearFlags & 0x400000) != 0);
    }

    // SST
    if (m_CurrentGearFlags & 0x40000)
    {
        if (mcCarSSTurbo* turbo = m_Entity->m_Car.m_CarSim->m_SSTurbo) turbo->sub_4D4270();
    }

    // Nitro
    if (m_CurrentGearFlags & 0x80000)
    {
        if (vehNitro* nitro = m_Entity->m_Car.m_CarSim->m_Nitro) nitro->sub_4D1F80();
    }

    // Damage / extra flags (high word usage)
    uint32_t highFlags = (m_CurrentGearFlags >> 16);

    if (highFlags & 0x10)
    {
        if (vehDamage* damage = m_Entity->m_Car.m_Damage)
        {
            uint32_t param = (highFlags >> 5) & 0xFFFFFF01;
            damage->sub_4CF500(param);
        }
    }
}

void vehInput::UpdateReplay()
{
    hook::Thunk<0x5684C0>::Call<void>(this);
}

void vehInput::ApplyReplayFrame()
{
    // hook::Thunk<0x46AB50>::Call<void>(this); // Call original

    // Inputs
    m_GasBrake = datReplay::GetByte() * 0.0039215689f;
    m_Brake = datReplay::GetByte() * 0.0039215689f;
    m_Handbrake = datReplay::GetByte() * 0.0039215689f;

    m_Steer = (int8_t)datReplay::GetByte() * 0.0078740157f;
    m_Drivable = (int8_t)datReplay::GetByte();

    uint16_t high = datReplay::ReadFrameUInt16();
    m_CurrentGearFlags = (m_CurrentGearFlags & 0x0000FFFF) | (high << 16);

    m_SteerRate = (int8_t)datReplay::GetByte() * 0.0078740157f;
    m_2WheelSteer = (int8_t)datReplay::GetByte() * 0.0078740157f;

    // Time
    uint32_t secondsBits = datReplay::GetInt();
    uint32_t elapsedBits = datReplay::GetInt();

    std::memcpy(&datTimeManager::Seconds, &secondsBits, sizeof(float));
    std::memcpy(&datTimeManager::ElapsedTime, &elapsedBits, sizeof(float));

    datTimeManager::InvSeconds = 1.0f / datTimeManager::Seconds;

    // Apply base state
    m_Entity->m_Car.m_CarSim->SetDrivable(m_Drivable);

    m_CarSim->m_Steer = m_Steer;
    m_CarSim->m_Throttle = m_GasBrake;
    m_CarSim->m_Brake = m_Brake;
    m_CarSim->m_Handbrake = m_Handbrake;

    // Replay transmission fix
    bool isAuto = m_CarSim->m_Transmission->m_Mode == TransmissionMode::Auto;

    if (!isAuto)
    {
        // Force the transmission gear to match the flag state
        int16_t gear = (int16_t)(m_CurrentGearFlags & 0xFFFF);
        m_CarSim->m_Transmission->SetCurrentGear(gear);
    }
    
    // Apply recorded inputs
    if (m_CurrentGearFlags & 0x1000000)
    {
        Printf("shift up\n");
        m_CarSim->m_Transmission->Upshift();
    }
    else if (m_CurrentGearFlags & 0x2000000)
    {
        Printf("shift down\n");
        m_CarSim->m_Transmission->Downshift();
    }

    if (!isAuto)
    {
        // Re-sync the flag's gear value with the transmission
        m_CurrentGearFlags = (m_CurrentGearFlags & 0xFFFF0000) | (m_CarSim->m_Transmission->m_CurrentGear & 0xFFFF);
    }

    // Visual / audio
    m_Entity->m_Car.m_Model->sub_4C4BC0((m_CurrentGearFlags & 0x20000) != 0);
    m_Entity->m_Car.m_Audio->sub_4D6800((m_CurrentGearFlags & 0x400000) != 0);

    // SST
    if (m_CurrentGearFlags & 0x40000)
    {
        mcCarSSTurbo*  ssturbo = m_Entity->m_Car.m_CarSim->m_SSTurbo;
        if (ssturbo) ssturbo->sub_4D4270();
    }

    // Nitro
    if (m_CurrentGearFlags & 0x80000)
    {
        vehNitro* nitro = m_Entity->m_Car.m_CarSim->m_Nitro;
        if (nitro) nitro->sub_4D1F80();
    }

    // Damage
    uint32_t highFlags = (m_CurrentGearFlags >> 16);

    if (highFlags & 0x10)
    {
        vehDamage* damage = m_Entity->m_Car.m_Damage;
        if (damage)
        {
            uint32_t param = (highFlags >> 5) & 0xFFFFFF01;
            damage->sub_4CF500(param);
        }
    }
}

float vehInput::sub_46AA90(float speed)
{
    return hook::Thunk<0x46AA90>::Call<float>(this, speed);
}

void vehInput::sub_46A760(int a2, int a3)
{
    hook::Thunk<0x46A760>::Call<void>(this, a2, a3);
}

void vehInput::sub_46A7A0(float steer, float* gasbrake, float* brake, int* drivable)
{
    hook::Thunk<0x46A7A0>::Call<void>(this, steer, gasbrake, brake, drivable);
}

void vehInput::SomethingReplay()
{   
    hook::Thunk<0x568460>::Call<void>(this);
}
