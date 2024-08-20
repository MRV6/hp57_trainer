#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_stdlib.h"
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
_testFunc testFunc;

int moneyToGive = 0;
std::string entityListQuery;
bool onlyLoadedModels = false;

void LoadAddresses()
{
    baseAddress = (uintptr_t)GetModuleHandle(NULL);

    studsAddress = GetPointerAddress(baseAddress + 0x00C5B600, { 0 });

    localPlayerAddressPtr = GetPointerAddress(baseAddress + 0x00003F18, { 0 });
    playerCharacterAddress = *(uintptr_t*)localPlayerAddressPtr + 0xFAC;

    worldAddress = 0x00F06ED0;

    setPlayerEntityIndex = (_setPlayerEntityIndex)(0x00748CF0);
    getUnkEntityValue = (_getUnkEntityValue)(0x00877C20);
    testFunc = (_testFunc)(0x007338C0);

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
        //*(int*)gameFocusPtr = 1; // Force game to render even when not focused

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

    if (ImGui::CollapsingHeader("DEBUG: Models"))
    {
        ImGui::Text("Max entity index: %i", maxEntityId);

        ImGui::InputText("Search", &entityListQuery);
        ImGui::Checkbox("Show only loaded models", &onlyLoadedModels);

        for (int i = 0; i < maxEntityId; i++)
        {
            uintptr_t entityAddress = *(uintptr_t*)(entitiesList + 4 * i);
            uintptr_t entityNameAddress = *(uintptr_t*)(entityAddress + 0x1C);
            uintptr_t entityLabelAddress = *(uintptr_t*)(entityAddress + 0xC);

            char* entityLabel = (char*)entityLabelAddress;
            char* entityName = (char*)entityNameAddress;
            
            std::string entityNameStr = std::string(entityName);
            std::string query = entityListQuery.c_str();
            std::transform(entityNameStr.begin(), entityNameStr.end(), entityNameStr.begin(), ::tolower);
            std::transform(query.begin(), query.end(), query.begin(), ::tolower);

            if ((query != "" && (entityNameStr.find(query) == std::string::npos)))
            {
                continue;
            }


            int unkEntityValue = getUnkEntityValue(*(uintptr_t*)worldAddress, i, 0xE);
            bool isLoaded = unkEntityValue != 0;

            if (onlyLoadedModels && !isLoaded)
            {
                continue;
            }

            const char* displayLabel = (std::string(entityLabel).find("TEXT STRING ERROR") != std::string::npos) ? "NO LABEL" : entityLabel;
            const char* loadedText = isLoaded ? "Loaded" : "Not loaded";

            ImGui::Text("%x: %s (%s, %i, %s, %i)", entityAddress, entityName, displayLabel, i, loadedText, unkEntityValue);

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
    /*uintptr_t localPlayer = *(uintptr_t*)localPlayerAddressPtr;

    ImGui::Text("X: %f", *(float*)(localPlayer + 0x24));
    ImGui::Text("Y: %f", *(float*)(localPlayer + 0x2C));
    ImGui::Text("Z: %f", *(float*)(localPlayer + 0x28));*/
    
    /*if (ImGui::Button("Test"))
    {
        cout << testFunc(*(uintptr_t*)localPlayerAddressPtr, 9) << endl;
    }*/

    RenderEntityList();

    // Give studs
    if (ImGui::CollapsingHeader("Cheats"))
    {
        ImGui::InputInt("Amount", &moneyToGive, 100, 1000);

        if (ImGui::Button("Give studs"))
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