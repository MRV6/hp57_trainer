#pragma once
void RenderImGuiItems();

typedef int(__cdecl* _setPlayerEntityIndex)(int localPlayer, int entityIndex, int oldEntityIndex, int unusedInt, bool forceSet, bool unkBool);
typedef int(__thiscall* _getUnkEntityValue)(uintptr_t worldClass, int entityIndex, int unk);
typedef int(__fastcall* _loadFunc)(uintptr_t charDefGameData);
typedef int(__fastcall* _deleteGameObject)(int triggerManager, uintptr_t unk, uintptr_t gameObjectAddress);

class GameObject
{
public:
    char pad_0000[8]; //0x0000
    uintptr_t* child; //0x0008
    char pad_000C[8]; //0x000C
    void* model; //0x0014
    char pad_0018[12]; //0x0018
    float X; //0x0024
    float Z; //0x0028
    float Y; //0x002C
    char pad_0030[3804]; //0x0030
    int32_t health; //0x0F0C
    int32_t maxHealth; //0x0F10
    char pad_0F14[1392]; //0x0F14
};