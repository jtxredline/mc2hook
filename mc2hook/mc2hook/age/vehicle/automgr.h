#pragma once
#include <mc2hook\mc2hook.h>

class mcCar;
class vehCarSim;
//class vehEntity;

class vehAutoMgr
{
public:
    void* m_Vtable;
    int m_NumCars;
    mcCar** m_Car;
    int m_NumSims;
    vehCarSim** m_Sim;
    int m_NumEngines;
    void* m_Engine;
    int m_NumAeros;
    void* m_Aero;
    int m_NumFluids;
    void* m_Fluid;
    int m_NumTransmissions;
    void* m_Transmission;
    int m_NumDrivetrains;
    void* m_Drivetrain;
    int m_NumWheels;
    void* m_Wheel;
    int m_NumSuspensions;
    void* m_Suspension;
    int m_NumAxles;
    void* m_Axle;
    int m_NumStucks;
    void* m_Stuck;
    int m_NumGyros;
    void* m_Gyro;
    int m_NumInputs;
    void* m_Input;
    int dword_6c;
    int dword_70;
    int m_NumDrivers;
    void* m_Driver;
    int dword_7c;
    int m_NumDamages;
    int dword_84;
    int m_NumAudios;
    int dword_8c;
    int m_NumModels;
    int dword_94;
    int dword_98;
    int dword_9c;
public:
    static hook::Type<vehAutoMgr*> Instance;
public:
    vehAutoMgr* GetInstance();
    mcCar* GetCar() { return *m_Car; }
    vehCarSim* GetSim() { return *m_Sim; }

    int ManagerAddEntry(mcCar* a2);
    //int AddEntry(int a2);
};
