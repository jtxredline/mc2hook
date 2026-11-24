#pragma once

class vehEntity;
//class mcPlayerInput;
//class mcCarSim;

class mcGhostCar
{
public:
    vehEntity* m_Entity = nullptr;

    bool Spawn(const char* carName);
    void Destroy();//(bool a2);

    void Update();

    void MakeSim(const char* carName);
    void MakeModel(const char* carName);

    //mcPlayerInput* GetInput();
    //mcCarSim* GetSim();
};
