#include "mcdata.h"
#include <mc2hook\mc2hook.h>
#include <age/core/output.h>
#include <age/types.h>
#include <age/globals.h>

declfield(mcData::Instance)(0x6C2E88);

void mcData::SetTOD(const char* todName)
{
    int todID = 0;

    while (_stricmp(g_TODList[todID], todName) != 0)
    {
        ++todID;

        if (todID >= 3)
        {
            Quitf("mcData: Unknown TOD!");
            return;
        }
    }

    m_TODID = todID;
}

void mcData::SetWeather(const char* weatherName)
{
    int weatherID = 0;

    while (_stricmp(g_WeatherList[weatherID], weatherName) != 0)
    {
        ++weatherID;

        if (weatherID >= 3)
        {
            Quitf("mcData: Unknown Weather!");
            return;
        }
    }

    m_WeatherID = weatherID;
}
