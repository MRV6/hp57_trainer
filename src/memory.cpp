#include <Windows.h>
#include <Psapi.h>
#include <vector>
#include <iostream>

uintptr_t GetPointerAddress(uintptr_t pointer, std::vector<uintptr_t> offsets)
{
    uintptr_t address = pointer;

    if (IsBadReadPtr(reinterpret_cast<PVOID>(address), sizeof(PVOID)))
    {
        return 0;
    }

    for (int i = 0; i < offsets.size(); i++) {
        address = *(uintptr_t*)address;
        address += offsets[i];

        if (IsBadReadPtr(reinterpret_cast<PVOID>(address), sizeof(PVOID)))
        {
            return 0;
        }
    }

    return address;
}