#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <format>

#include "../vendor/imgui/imgui.h"

#include "imgui_menu.h"
#include "memory.h"

#include "main.h"
#include "game_objects_list.h"
#include "models_list.h"
#include "logs.h"

#define ENABLE_CONSOLE false

uintptr_t baseAddress;
uintptr_t studsAddress;
uintptr_t modelsClassAddress;
uintptr_t gameFocusPtr;
//uintptr_t alphaAddress;
uintptr_t harryGameObjectPtr;

_setPlayerModelIndex setPlayerModelIndex;
_getCharDefGameData getCharDefGameData;
_deleteGameObject deleteGameObject;

int studsToGive = 0;
float localPlayerAlpha = 1.0;
bool maxHealth = false;

bool showCheats = false;
bool showLocalPlayerInfos = false;

// TODO: Check if pointers are valid before trying to read them to fix crashes
// TODO: Make all of this in separate files using an event manager (HP57::OnTick, HP57::OnLoad, HP57::OnStop ...)

void LoadAddresses()
{
    baseAddress = (uintptr_t)GetModuleHandle(NULL);

    studsAddress = GetPointerAddress(baseAddress + 0x00C5B600, { 0 });

    modelsClassAddress = 0x00F06ED0;

    setPlayerModelIndex = (_setPlayerModelIndex)(0x00748CF0);
    getCharDefGameData = (_getCharDefGameData)(0x00877C20);
    deleteGameObject = (_deleteGameObject)(0x00648600);

    gameFocusPtr = GetPointerAddress(baseAddress + 0x00189634, { 0 }); // A byte, 0 = game not focused, 1 = game focused

    harryGameObjectPtr = GetPointerAddress(baseAddress + 0x00003F18, { 0 });

    Logs::Push("Addresses loaded !\n");
}

void GameLoop()
{
    *(int*)gameFocusPtr = 1; // Force game to render even when not focused

    uintptr_t harryGameObjectAddress = *(uintptr_t*)harryGameObjectPtr;

    if (harryGameObjectAddress)
    {
        GameObject* harryGameObject = (GameObject*)harryGameObjectAddress;

        if (maxHealth)
        {
            harryGameObject->health = harryGameObject->maxHealth;
        }

        harryGameObject->unkChildClass->alpha = localPlayerAlpha;
    }
}

void MainThread(HMODULE hModule)
{
#if ENABLE_CONSOLE
    FILE* f;
    AllocConsole();
    freopen_s(&f, "CONOUT$", "w", stdout);
#endif

    LoadAddresses();

    InitImgui();

#if ENABLE_CONSOLE
    if (f != 0) fclose(f);
    FreeConsole();
#endif

    MessageBeep(MB_OK);
    FreeLibraryAndExitThread(hModule, 0);
}

void RenderCheats()
{
    ImGui::Begin("Cheats");

    // Give studs
    ImGui::InputInt("Amount", &studsToGive, 100, 1000);

    if (ImGui::Button("Give studs"))
    {
        *(int*)studsAddress += studsToGive;
        Logs::Push("%d studs added !\n", studsToGive);
    }

    // Max health
    ImGui::Checkbox("Max health", &maxHealth);

    // Alpha
    ImGui::SliderFloat("Player opacity", &localPlayerAlpha, 0.0, 1.0, "%.1f");

    ImGui::End();
}

void RenderLocalPlayerInfos()
{
    ImGui::Begin("Local player infos");

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

    ImGui::End();
}

void RenderImGuiItems()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Tools"))
        {
            ImGui::Checkbox("Show models list", &showModelsList);
            ImGui::Checkbox("Show game objects list", &showGameObjectsList);
            ImGui::Checkbox("Show cheats", &showCheats);
            ImGui::Checkbox("Show local player infos", &showLocalPlayerInfos);
            ImGui::Checkbox("Show logs", &Logs::Visible);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (showModelsList) RenderModelsList();
    if (showGameObjectsList) RenderGameObjectsList();
    if (showCheats) RenderCheats();
    if (showLocalPlayerInfos) RenderLocalPlayerInfos();
    if (Logs::Visible) Logs::Draw();
}


BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(hModule);
            HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, NULL, NULL);

            if (hThread != 0)
            {
                CloseHandle(hThread);
            }
        }
    }

    return TRUE;
}