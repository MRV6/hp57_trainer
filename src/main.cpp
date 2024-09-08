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
uintptr_t playerGameObjectPtr;
uintptr_t harryGameAddress;
uintptr_t triggerManagerAddress;

_setPlayerModelIndex setPlayerModelIndex;
_getCharDefGameData getCharDefGameData;
_deleteGameObject deleteGameObject;
_loadModelByIndex loadModelByIndex;

int studsToGive = 0;
float localPlayerAlpha = 1.0;
bool infiniteHealth = false;

bool showCheats = false;
bool showLocalPlayerInfos = false;

// TODO: Make all of this in separate files using an event manager (HP57::OnTick, HP57::OnLoad, HP57::OnStop ...)

static void LoadAddresses()
{
    baseAddress = (uintptr_t)GetModuleHandle(NULL);

    modelsClassAddress = 0x00F06ED0;
    studsAddress = GetPointerAddress(baseAddress + 0x00C5B600, { 0 });
    gameFocusPtr = GetPointerAddress(baseAddress + 0x00189634, { 0 }); // A byte, 0 = game not focused, 1 = game focused
    playerGameObjectPtr = GetPointerAddress(baseAddress + 0x00003F18, { 0 });
    harryGameAddress = GetPointerAddress(baseAddress + 0x00956834, { 4, 0 });

    uintptr_t levelContainer = baseAddress + 0x00C48AEC;
    triggerManagerAddress = GetPointerAddress(levelContainer, { 0x120 });

    setPlayerModelIndex = (_setPlayerModelIndex)(0x00748CF0);
    getCharDefGameData = (_getCharDefGameData)(0x00877C20);
    deleteGameObject = (_deleteGameObject)(0x00654620);
    loadModelByIndex = (_loadModelByIndex)(0x00483050);

    Logs::Push("Addresses loaded !\n");
}

void GameLoop()
{
    *(int*)gameFocusPtr = 1; // Force game to render even when not focused

    // Player
    auto playerGameObject = GetPlayerGameObject();

    if (playerGameObject)
    {
        if (infiniteHealth)
        {
            playerGameObject->health = 2056;
        }

        playerGameObject->unkChildClass->alpha = localPlayerAlpha;
    }

    // Game objects
    auto allGameObjects = GetAllGameObjects();

    for (int i = 0; i < allGameObjects.size(); i++)
    {
        auto gameObject = allGameObjects[i];
        gameObject->unkChildClass->alpha = gameObjectsAlpha;
    }
}

void RenderCheats()
{
    ImGui::Begin("Cheats");

    // Give studs
    ImGui::InputInt("##amount", &studsToGive, 100, 1000);

    ImGui::SameLine();
    if (ImGui::Button("Give studs"))
    {
        *(int*)studsAddress += studsToGive;
        Logs::Push("%d studs added !\n", studsToGive);
    }

    // Infinite health
    ImGui::Checkbox("Infinite health", &infiniteHealth);

    // Alpha
    ImGui::SliderFloat("Player opacity", &localPlayerAlpha, 0.0, 1.0, "%.1f");

    ImGui::End();
}

void RenderLocalPlayerInfos()
{
    ImGui::Begin("Local player infos");

    auto playerGameObject = GetPlayerGameObject();

    if (playerGameObject)
    {
        ImGui::Text("Game object address : %x", playerGameObject);
        ImGui::Text("X: %.3f", playerGameObject->X);
        ImGui::Text("Y: %.3f", playerGameObject->Y);
        ImGui::Text("Z: %.3f", playerGameObject->Z);
        ImGui::Text("Character model: %s", (char*)(playerGameObject->model));
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