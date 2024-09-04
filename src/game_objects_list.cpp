#include "../vendor/imgui/imgui.h"
#include "memory.h"
#include <string>
#include <format>
#include "main.h"
#include "logs.h"

bool showGameObjectsList = false;
float npcsAlpha = 1.0f;

void RenderGameObjectsList()
{
    ImGui::Begin("Game objects list");

    ImGui::SliderFloat("Game objects opacity", &npcsAlpha, 0.0, 1.0, "%.1f");

    uintptr_t levelContainer = baseAddress + 0x00C48AEC;
    uintptr_t triggerManagerAddr = GetPointerAddress(levelContainer, { 0x120 });
    uintptr_t triggerManager = *(uintptr_t*)triggerManagerAddr;

    if (triggerManager)
    {
        int gameObjectsListOffset = 0x434;
        uintptr_t gameObjectsList = (triggerManager + gameObjectsListOffset);
        int gameObjectsCount = 0;

        for (int i = 0; i < 32; i++) // Game internally use 128 to loop so we can up this loop if needed
        {
            uintptr_t gameObjectAddress = *(uintptr_t*)(gameObjectsList + 4 * i);

            if (gameObjectAddress != 0)
            {
                GameObject* gameObject = (GameObject*)gameObjectAddress;

                gameObject->unkChildClass->alpha = npcsAlpha;

                ImGui::PushID(i);
                std::string objectAddressStr = std::format("{:x}: {:s}", gameObjectAddress, (char*)gameObject->model);

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
                        uintptr_t harryGameObjectAddress = *(uintptr_t*)harryGameObjectPtr;

                        if (harryGameObjectAddress)
                        {
                            GameObject* harryGameObject = (GameObject*)harryGameObjectAddress;

                            uintptr_t characterPhantomEntity = GetPointerAddress(baseAddress + 0x00C58348, { 0 });
                            CharacterPhantomEntity* charPhantomEntity = (CharacterPhantomEntity*)characterPhantomEntity;

                            if (harryGameObject && charPhantomEntity)
                            {
                                float z = gameObject->Z + 0.5;

                                // The game expects us to set the position on both of these classes
                                harryGameObject->X = gameObject->X;
                                harryGameObject->Y = gameObject->Y;
                                harryGameObject->Z = z;

                                charPhantomEntity->X = gameObject->X;
                                charPhantomEntity->Y = gameObject->Y;
                                charPhantomEntity->Z = z;

                                Logs::Push("Teleported to %x\n", gameObjectAddress);
                            }
                        }
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
                gameObjectsCount++;
            }
        }

        ImGui::Text("Game objects count: %d", gameObjectsCount);
    }

    ImGui::End();
}