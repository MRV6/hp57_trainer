#pragma once
#include <cstdint>
void GameLoop();
void RenderImGuiItems();

extern uintptr_t baseAddress;
extern uintptr_t playerGameObjectPtr;
extern uintptr_t modelsClassAddress;
extern uintptr_t harryGameAddress;
extern uintptr_t triggerManagerAddress;

typedef int(__cdecl* _setPlayerModelIndex)(int gameObject, int modelIndex, int oldModelIndex, int unusedInt, bool forceSet, bool unkBool);
typedef int(__thiscall* _getCharDefGameData)(uintptr_t modelsClass, int modelIndex, int unk);
typedef int(__thiscall* _deleteGameObject)(int triggerManager, int gameObjectAddress, bool a3);
typedef int(__thiscall* _loadModelByIndex)(int harryGame, int modelIndex, int anotherModelIndex);

extern _setPlayerModelIndex setPlayerModelIndex;
extern _getCharDefGameData getCharDefGameData;
extern _deleteGameObject deleteGameObject;
extern _loadModelByIndex loadModelByIndex;

class UnkChildClass
{
public:
    char pad_0000[2784]; //0x0000
    float alpha; //0x0AE0
    char pad_0AE4[1372]; //0x0AE4
};

class CharacterPhantomEntity
{
public:
    char pad_0000[112]; //0x0000
    float X; //0x0070
    float Z; //0x0074
    float Y; //0x0078
    char pad_007C[196]; //0x007C
};

class CharacterData
{
public:
    char pad_0000[4]; //0x0000
    int32_t skinIndex; //0x0004
    char pad_0008[4]; //0x0008
    void* labelPtr; //0x000C
    char pad_0010[8]; //0x0010
    void* pathPtr; //0x0018
    void* namePtr; //0x001C
    char pad_0020[48]; //0x0020
    uintptr_t charDefFile; //0x0050
    char pad_0054[2148]; //0x0054
};

class GameObject
{
public:
    char pad_0000[8]; //0x0000
    void* child; //0x0008
    char pad_000C[8]; //0x000C
    void* model; //0x0014
    char pad_0018[12]; //0x0018
    float X; //0x0024
    float Z; //0x0028
    float Y; //0x002C
    char pad_0030[248]; //0x0030
    void* brain; //0x0128
    CharacterPhantomEntity* phantomEntity; //0x012C
    UnkChildClass* unkChildClass; //0x0130
    char pad_0134[3516]; //0x0134
    int64_t modelIndex; //0x0EF0
    char pad_0EF8[20]; //0x0EF8
    int32_t health; //0x0F0C
    char pad_0F10[156]; //0x0F10
    CharacterData* modelClass; //0x0FAC
    char pad_0FB0[8]; //0x0FB0
    uintptr_t* charDefGameData; //0x0FB8
    char pad_0FBC[1240]; //0x0FBC
};