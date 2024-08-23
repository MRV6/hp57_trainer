#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <format>

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

// TODO: Check if pointers are valid before trying to read them to fix crashes

void LoadAddresses()
{
    baseAddress = (uintptr_t)GetModuleHandle(NULL);

    studsAddress = GetPointerAddress(baseAddress + 0x00C5B600, { 0 });

    localPlayerAddressPtr = GetPointerAddress(baseAddress + 0x00003F18, { 0 });
    playerCharacterAddress = *(uintptr_t*)localPlayerAddressPtr + 0xFAC;

    worldAddress = 0x00F06ED0;

    setPlayerEntityIndex = (_setPlayerEntityIndex)(0x00748CF0);
    getUnkEntityValue = (_getUnkEntityValue)(0x00877C20);
    loadFunc = (_loadFunc)(0x007B9D20);

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
    if (ImGui::CollapsingHeader("DEBUG: Models"))
    {
        uintptr_t world = *(uintptr_t*)worldAddress;
        uintptr_t modelsList = *(uintptr_t*)(world + 0x64);
        int maxModelId = *(int*)(world + 0x30);

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
                    //cout << "Char def: " << *(uintptr_t*)(modelAddress + 0x50) << endl;
                    cout << loadFunc(modelAddress) << endl;
                }
                ImGui::PopID();
            }
        }
    }
}

void RenderGameObjectsList()
{
    if (ImGui::CollapsingHeader("DEBUG: Game objects list"))
    {
        uintptr_t gameObjectsList = GetPointerAddress(baseAddress + 0x00C48AEC, { 0x120, 0x434 });

        if (gameObjectsList)
        {
            for (int i = 0; i < 32; i++)
            {
                uintptr_t gameObjectAddress = *(uintptr_t*)(gameObjectsList + 4 * i);

                if (gameObjectAddress != 0)
                {
                    GameObject* gameObject = (GameObject*)gameObjectAddress;

                    ImGui::PushID(i);
                    ImGui::SetNextItemOpen(true);

                    std::string objectAddressStr = std::format("{:x}", gameObjectAddress);

                    if (ImGui::TreeNode(objectAddressStr.c_str()))
                    {
                        ImGui::Text("Model : %s", (char*)(gameObject->model));
                        ImGui::Text("X: %.3f", gameObject->X);
                        ImGui::Text("Y: %.3f", gameObject->Y);
                        ImGui::Text("Z: %.3f", gameObject->Z);
                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }
            }
        }
    }
}

void RenderImGuiItems()
{
    if (ImGui::CollapsingHeader("DEBUG: Character infos"))
    {
        uintptr_t harryGameObjectAddress = *(uintptr_t*)harryGameObjectPtr;

        if (harryGameObjectAddress)
        {
            GameObject* harryGameObject = (GameObject*)harryGameObjectAddress;

            ImGui::Text("Game object address : %x", harryGameObjectAddress);
            ImGui::Text("X: %.3f", harryGameObject->X);
            ImGui::Text("Y: %.3f", harryGameObject->Y);
            ImGui::Text("Z: %.3f", harryGameObject->Z);
            ImGui::Text("Character model: %s", (char*)(harryGameObject->model));
        }
    }

    RenderModelsList();
    RenderGameObjectsList();

    if (ImGui::CollapsingHeader("Cheats"))
    {
        // Give studs
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