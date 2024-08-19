#include <windows.h>
#include <iostream>
#include <string>
#include "../vendor/imgui/imgui.h"
#include "imgui_menu.h"
#include "memory.h"
#include "hp57_trainer.h"

using namespace std;

uintptr_t baseAddress;
uintptr_t studsAddress;
uintptr_t localPlayerAddressPtr;

uintptr_t playerCharacterAddress;
uintptr_t worldAddress;
uintptr_t entityList;

uintptr_t gameFocusPtr;

_setPlayerEntityIndex setPlayerEntityIndex;
_getUnkEntityValue getUnkEntityValue;

int moneyToGive = 0;

void LoadAddresses()
{
    baseAddress = (uintptr_t)GetModuleHandle(NULL);

    studsAddress = GetPointerAddress(baseAddress + 0x00C5B600, { 0 });

    localPlayerAddressPtr = GetPointerAddress(baseAddress + 0x00003F18, { 0 });
    playerCharacterAddress = *(uintptr_t*)localPlayerAddressPtr + 0xFAC;

    worldAddress = 0x00F06ED0;

    setPlayerEntityIndex = (_setPlayerEntityIndex)(0x00748CF0);
    getUnkEntityValue = (_getUnkEntityValue)(0x00877C20);

    gameFocusPtr = GetPointerAddress(baseAddress + 0x00189634, { 0 }); // A byte, 0 = game not focused, 1 = game focused
}

void SetPlayerEntityIndex(int index)
{
    cout << "Set player entity index to " << index << endl;
    int success = setPlayerEntityIndex(*(uintptr_t*)localPlayerAddressPtr, index, 550, 1, false, true);
    cout << "Set player entity index result " << success << endl;
}

void MainThread(HMODULE hModule)
{
    FILE* f;
    AllocConsole();
    freopen_s(&f, "CONOUT$", "w", stdout);

    LoadAddresses();
    InitImgui();

    std::cout << "Trainer ready" << std:: endl;

    while (true)
    {
        *(int*)gameFocusPtr = 1; // Force game to render even when not focused

        DrawImgui();

        if (GetAsyncKeyState(VK_F3) & 1)
        {
            ToggleMenu();
        }

        if (GetAsyncKeyState(VK_END) & 1)
        {
            break;
        }
    }

    std::cout << "Exiting trainer ..." << std::endl;

    CleanupImgui();

    Sleep(500); // Sleep before unloading DLL
    MessageBeep(MB_OK);

    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
}

void RenderEntityList()
{
    uintptr_t world = *(uintptr_t*)worldAddress;
    uintptr_t entitiesList = *(uintptr_t*)(world + 0x64);
    int maxEntityId = *(int*)(world + 0x30);

    ImGui::Text("Max entity id: %i", maxEntityId);

    int modelsCount = maxEntityId / 4;

    if (ImGui::CollapsingHeader("DEBUG: Models"))
    {
        for (int i = 0; i < maxEntityId; i++)
        {
            uintptr_t entity = *(uintptr_t*)(entitiesList + 4 * i);
            uintptr_t entityNameAddress = *(uintptr_t*)(entity + 0x1C);
            uintptr_t entityLabelAddress = *(uintptr_t*)(entity + 0xC);

            char* entityLabel = (char*)entityLabelAddress;

            bool isLabelValid = std::string(entityLabel).find("TEXT STRING ERROR") != std::string::npos;

            int unkEntityValue = getUnkEntityValue(*(uintptr_t*)worldAddress, i, 0xE);
            bool isLoaded = unkEntityValue != 0;
            const char* loadedText = isLoaded ? "Loaded" : "Not loaded";

            if (isLabelValid)
            {
                ImGui::Text("%s (%i, %s)", (char*)entityNameAddress, i, loadedText);
            }
            else
            {
                ImGui::Text("%s (%s, %i, %s)", (char*)entityNameAddress, entityLabel, i, loadedText);
            }

            if (isLoaded)
            {
                ImGui::SameLine();
                ImGui::PushID(i);
                if (ImGui::Button("Swap to"))
                {
                    SetPlayerEntityIndex(i);
                }
                ImGui::PopID();
            }
        }
    }
}

void RenderImGuiItems()
{
    RenderEntityList();

    // Give studs
    if (ImGui::CollapsingHeader("Cheats"))
    {
        ImGui::InputInt("Montant", &moneyToGive, 100, 1000);

        if (ImGui::Button("Give argent"))
        {
            *(int*)studsAddress += moneyToGive;
        }
    }
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(hModule);
            HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, NULL, NULL);

            CloseHandle(hThread);
        }
        case DLL_PROCESS_DETACH:
        {
            break;
        }
    }

    return TRUE;
}

int main()
{
    return 0;
}