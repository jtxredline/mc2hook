#pragma once
#include <mc2hook\mc2hook.h>

class mcData {
public:
    int m_CityID;
    int m_TODID;
    int m_WeatherID;
    int dword_0c;
    int dword_10;
    int dword_14;
    int m_RaceTypeID;
    int m_WeaponModeID;
    int m_MultiplayerTeamModeID;
    int dword_24;
    int dword_28;
    int dword_2c;
    int dword_30;
    int dword_34;
    int dword_38;
    int dword_3c;
    int dword_40;
    int m_LayoutID;
    int dword_48;
    int dword_4c;

public:
    static hook::Type<mcData*> Instance;

public:
    // Default values when loading levels from .bat
    void SetTOD(const char* todName);
    void SetWeather(const char* weatherName);
};
