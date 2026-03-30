#pragma once

class mcConfig {
public:
    // These seem to be responsible for the default values in the menu
    static int LookupTOD(const char* todName);
    static int LookupWeather(const char* weatherName);
};
