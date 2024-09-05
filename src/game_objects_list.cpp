#include "../vendor/imgui/imgui.h"
#include "memory.h"
#include <string>
#include <format>
#include "main.h"
#include "logs.h"

bool showGameObjectsList = false;
float npcsAlpha = 1.0f;

std::vector<GameObject*> GetAllGameObjects()
{
    std::vector<GameObject*> allGameObjects;

    uintptr_t levelContainer = baseAddress + 0x00C48AEC;
    uintptr_t triggerManagerAddr = GetPointerAddress(levelContainer, { 0x120 });
    uintptr_t triggerManager = *(uintptr_t*)triggerManagerAddr;

    if (triggerManager)
    {
        int gameObjectsListOffset = 0x434;
        uintptr_t gameObjectsList = (triggerManager + gameObjectsListOffset);

        for (int i = 0; i < 32; i++) // Game internally use 128 to loop so we can up this loop if needed
        {
            uintptr_t gameObjectAddress = *(uintptr_t*)(gameObjectsList + 4 * i);

            if (gameObjectAddress != 0)
            {
                GameObject* gameObject = (GameObject*)gameObjectAddress;

                allGameObjects.push_back(gameObject);
            }
        }
    }

    return allGameObjects;
}

GameObject* GetPlayerGameObject()
{
    uintptr_t playerGameObjectAddress = *(uintptr_t*)playerGameObjectPtr;

    if (playerGameObjectAddress)
    {
        GameObject* playerGameObject = (GameObject*)playerGameObjectAddress;

        if (playerGameObject)
        {
            return playerGameObject;
        }
    }

    return nullptr;
}

static void SetGameObjectCoords(GameObject* gameObject, float x, float y, float z)
{
    // The game expects us to set the position on both of these classes
    gameObject->X = x;
    gameObject->Y = y;
    gameObject->Z = z;

    gameObject->phantomEntity->X = x;
    gameObject->phantomEntity->Y = y;
    gameObject->phantomEntity->Z = z;
}

static void TeleportToGameObject(GameObject* gameObject)
{
    auto playerGameObject = GetPlayerGameObject();

    if (playerGameObject)
    {
        float z = gameObject->Z + 0.5;
        SetGameObjectCoords(playerGameObject, gameObject->X, gameObject->Y, z);

        Logs::Push("Teleported to %x\n", gameObject);
    }
}

static void BringGameObject(GameObject* gameObject)
{
    auto playerGameObject = GetPlayerGameObject();

    if (playerGameObject)
    {
        float z = playerGameObject->Z + 0.5;
        SetGameObjectCoords(gameObject, playerGameObject->X, playerGameObject->Y, z);

        Logs::Push("Brought %x\n", gameObject);
    }
}

static void BringAllGameObjects()
{
    auto allGameObjects = GetAllGameObjects();

    for (int i = 0; i < allGameObjects.size(); i++)
    {
        auto gameObject = allGameObjects[i];

        BringGameObject(gameObject);
    }
}

void RenderGameObjectsList()
{
    ImGui::Begin("Game objects list");

    ImGui::SliderFloat("Game objects opacity", &npcsAlpha, 0.0, 1.0, "%.1f");

    if (ImGui::Button("Bring all"))
    {
        BringAllGameObjects();
    }

    auto allGameObjects = GetAllGameObjects();

    int gameObjectsCount = allGameObjects.size();
    ImGui::Text("Game objects count: %i", gameObjectsCount);

    for (int i = 0; i < gameObjectsCount; i++)
    {
        auto gameObject = allGameObjects[i];

        gameObject->unkChildClass->alpha = npcsAlpha;

        ImGui::PushID(i);

        std::string objectAddressStr = std::format("{:x}: {:s}", reinterpret_cast<uintptr_t>(gameObject), (char*)gameObject->model);

        if (ImGui::TreeNode(objectAddressStr.c_str()))
        {
            ImGui::Text("X: %.3f", gameObject->X);
            ImGui::Text("Y: %.3f", gameObject->Y);
            ImGui::Text("Z: %.3f", gameObject->Z);
            ImGui::Text("Child address: %x", gameObject->child);
            ImGui::Text("Health: %i", gameObject->health);

            // Somehow this functions does not really delete the game object
            // It just removes it from the game objects list but the entity is still physically here
            // However, if we interact with the entity (collision) it will re-appear on the list
            // Needs to figure that out
            /*if (ImGui::Button("Delete"))
            {
                deleteGameObject(triggerManager, gameObjectAddress, true);
            }*/

            if (ImGui::Button("Teleport to"))
            {
                TeleportToGameObject(gameObject);
            }

            if (ImGui::Button("Bring to me"))
            {
                BringGameObject(gameObject);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::End();
}