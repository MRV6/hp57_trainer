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

uintptr_t gameFocusPtr;
//uintptr_t alphaAddress;
uintptr_t harryGameObjectPtr;

_setPlayerEntityIndex setPlayerEntityIndex;
_getUnkEntityValue getUnkEntityValue;
_loadFunc loadFunc;

std::string modelListQuery;
int moneyToGive = 0;
bool onlyLoadedModels = false;
float localPlayerAlpha = 1.0;

// TODO: Cleanup code and make some structs

void LoadAddresses()
{
    baseAddress = (uintptr_t)GetModuleHandle(NULL);

    studsAddress = GetPointerAddress(baseAddress + 0x00C5B600, { 0 });

    localPlayerAddressPtr = GetPointerAddress(baseAddress + 0x00003F18, { 0 });
    playerCharacterAddress = *(uintptr_t*)localPlayerAddressPtr + 0xFAC;

    worldAddress = 0x00F06ED0;

    setPlayerEntityIndex = (_setPlayerEntityIndex)(0x00748CF0);
    getUnkEntityValue = (_getUnkEntityValue)(0x00877C20);
    loadFunc = (_loadFunc)(0x006D61F0);

    gameFocusPtr = GetPointerAddress(baseAddress + 0x00189634, { 0 }); // A byte, 0 = game not focused, 1 = game focused

    //alphaAddress = GetPointerAddress(baseAddress + 0x00C53930, { 0x130, 0xAE0 });
    harryGameObjectPtr = GetPointerAddress(baseAddress + 0x00003F18, { 0 });
}

void SetPlayerEntityIndex(int index)
{
    cout << "Set player entity index to " << index << endl;
    int success = setPlayerEntityIndex(*(uintptr_t*)localPlayerAddressPtr, index, 550, 1, false, true);
    cout << "Set player entity index result " << success << endl;
}

void HandleData()
{
    *(int*)gameFocusPtr = 1; // Force game to render even when not focused

    // Alpha address if dynamic, re-add it later once we found a way to get the parent
    //*(float*)alphaAddress = localPlayerAlpha;
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

        DrawImgui();
        HandleData();

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

void RenderModelsList()
{
    uintptr_t world = *(uintptr_t*)worldAddress;
    uintptr_t modelsList = *(uintptr_t*)(world + 0x64);
    int maxModelId = *(int*)(world + 0x30);

    if (ImGui::CollapsingHeader("DEBUG: Models"))
    {
        ImGui::Text("Max model index: %i", maxModelId);

        ImGui::InputText("Search", &modelListQuery);
        ImGui::Checkbox("Show only loaded models", &onlyLoadedModels);

        for (int i = 0; i < maxModelId; i++)
        {
            uintptr_t modelAddress = *(uintptr_t*)(modelsList + 4 * i);
            uintptr_t modelNameAddress = *(uintptr_t*)(modelAddress + 0x1C);
            uintptr_t modelLabelAddress = *(uintptr_t*)(modelAddress + 0xC);

            char* modelLabel = (char*)modelLabelAddress;
            char* modelName = (char*)modelNameAddress;
            
            std::string modelNameStr = std::string(modelName);
            std::string query = modelListQuery.c_str();
            std::transform(modelNameStr.begin(), modelNameStr.end(), modelNameStr.begin(), ::tolower);
            std::transform(query.begin(), query.end(), query.begin(), ::tolower);

            if ((query != "" && (modelNameStr.find(query) == std::string::npos)))
            {
                continue;
            }


            int unkModelValue = getUnkEntityValue(*(uintptr_t*)worldAddress, i, 0xE);
            bool isLoaded = unkModelValue != 0;

            if (onlyLoadedModels && !isLoaded)
            {
                continue;
            }

            const char* displayLabel = (std::string(modelLabel).find("TEXT STRING ERROR") != std::string::npos) ? "NO LABEL" : modelLabel;
            const char* loadedText = isLoaded ? "Loaded" : "Not loaded";

            ImGui::Text("%x: %s (%s, %i, %s, %i)", modelAddress, modelName, displayLabel, i, loadedText, unkModelValue);

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
            else
            {
                ImGui::SameLine();
                ImGui::PushID(i);
                if (ImGui::Button("Load"))
                {
                    //cout << "Char def: " << *(uintptr_t*)(entityAddress + 0x50) << endl;
                    cout << "Load level" << endl;
                    cout << loadFunc(*(uintptr_t*)0x006D61F0) << endl;
                }
                ImGui::PopID();
            }
        }
    }
}

void RenderImGuiItems()
{
    uintptr_t harryGameObjectAddress = *(uintptr_t*)harryGameObjectPtr;

    ImGui::Text("Game object address : %x", harryGameObjectAddress);
    ImGui::Text("X: %f", *(float*)(harryGameObjectAddress + 0x24));
    ImGui::Text("Y: %f", *(float*)(harryGameObjectAddress + 0x2C));
    ImGui::Text("Z: %f", *(float*)(harryGameObjectAddress + 0x28));
    ImGui::Text("Character model: %s", (char*)(*(uintptr_t*)(harryGameObjectAddress + 0x14)));

    RenderModelsList();

    if (ImGui::CollapsingHeader("Cheats"))
    {
        // Give studs
        ImGui::InputInt("Amount", &moneyToGive, 100, 1000);

        if (ImGui::Button("Give studs"))
        {
            *(int*)studsAddress += moneyToGive;
        }

        // Local player alpha
        // Alpha = game object + 130 + EF0
        /*ImGui::InputFloat("Player alpha", &localPlayerAlpha, 0.05, 0.1, "%.1f");

        if (localPlayerAlpha < 0.0)
        {
            localPlayerAlpha = 0.0;
        }
        else if (localPlayerAlpha > 1.0)
        {
            localPlayerAlpha = 1.0;
        }*/
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