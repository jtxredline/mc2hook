#include <age/vector/matrix34.h>
#include <stdio.h>
//#include <string.h>

bool GetPivotPrefix(Matrix34& out, const char* directory, const char* filename, const char* partname) {
    char fullname[64] = { 0 };
    snprintf(fullname, 64, "%s_%s", filename, partname);

    //Stream* fp = datAssetManager::open(directory, fullname, "mtx", 1, true);
    //if (!fp) return false;

    //fp->read(&out, sizeof(Matrix34));
    //delete fp;

    return true;
}