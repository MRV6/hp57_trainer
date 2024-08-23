#pragma once
void RenderImGuiItems();

typedef int(__cdecl* _setPlayerEntityIndex)(int localPlayer, int entityIndex, int oldEntityIndex, int unusedInt, bool forceSet, bool unkBool);
typedef int(__thiscall* _getUnkEntityValue)(uintptr_t worldClass, int entityIndex, int unk);
typedef int(__fastcall* _loadFunc)(uintptr_t charDefGameData);

class GameObject
{
public:
    char pad_0000[20]; //0x0000
    void* model; //0x0014
    char pad_0018[12]; //0x0018
    float X; //0x0024
    float Z; //0x0028
    float Y; //0x002C
    char pad_0030[80]; //0x0030
};