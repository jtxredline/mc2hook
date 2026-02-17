#pragma once
#include <string.h>
#include <age\memory\memory.h>

static char* StringDuplicate(const char* str)
{
    if (!str) return nullptr;
    int size = strlen(str) + 1;
    char* dup = (char*)age_allocate(size);
    memcpy(dup, str, size);
    return dup;
}

static char* StringToLower(const char* devName)
{
    static char buffer[256];

    int i = 0;
    while (devName[i] != '\0' && i < 255)
    {
        char c = devName[i];

        // ASCII uppercase -> lowercase
        if (c >= 'A' && c <= 'Z')
            c = c + ('a' - 'A');  // +32

        buffer[i++] = c;
    }

    buffer[i] = '\0';
    return buffer;
}
