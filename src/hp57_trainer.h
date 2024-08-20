#pragma once
void RenderImGuiItems();
typedef int(__cdecl* _setPlayerEntityIndex)(int localPlayer, int entityIndex, int oldEntityIndex, int unusedInt, bool forceSet, bool unkBool);
typedef int(__thiscall* _getUnkEntityValue)(uintptr_t worldClass, int entityIndex, int unk);
typedef int(__cdecl* _testFunc)(int player, int entityIndex);