#ifndef __KARBAZOL_DRAGNDROP_2_0__PROTECT_H__
#define __KARBAZOL_DRAGNDROP_2_0__PROTECT_H__

#include <windows.h>

class MemProtect
{
private:
    MEMORY_BASIC_INFORMATION _mi;
    DWORD _oldProtect;
public:
    MemProtect(void* p, DWORD protect);
    ~MemProtect();
};

#endif // __KARBAZOL_DRAGNDROP_2_0__PROTECT_H__

// vim: set et ts=4 :

