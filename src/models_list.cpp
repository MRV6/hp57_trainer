#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_stdlib.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <format>
#include "main.h"
#include "logs.h"

bool showModelsList = false;
std::string modelListQuery;

static void SetPlayerModelIndex(int index, const char* modelName)
{
    uintptr_t harryGameObjectAddress = *(uintptr_t*)playerGameObjectPtr;

    if (harryGameObjectAddress)
    {
        GameObject* harryGameObject = (GameObject*)harryGameObjectAddress;
        int success = setPlayerModelIndex(harryGameObjectAddress, index, harryGameObject->modelIndex, 1, false, false);

        if (success)
        {
            Logs::Push("Player model set to %s (index: %i)\n", modelName, index);
        }
        else
        {
            Logs::Push("Cannot set player model to %s (index: %i). Model is probably not loaded.\n", modelName, index);
        }
    }
}

static bool IsModelLoaded(int modelIndex)
{
    uintptr_t modelsClass = *(uintptr_t*)modelsClassAddress;
    int charDefGameData = getCharDefGameData(modelsClass, modelIndex, 0xE);
    return charDefGameData != 0;
}

void RenderModelsList()
{
    ImGui::Begin("Models list");

    uintptr_t modelsClass = *(uintptr_t*)modelsClassAddress;
    uintptr_t modelsList = *(uintptr_t*)(modelsClass + 0x64);

    int maxModelId = *(int*)(modelsClass + 0x30);
    ImGui::Text("Max model index: %i", maxModelId);

    ImGui::InputText("Search", &modelListQuery);

    for (int i = 0; i < maxModelId; i++)
    {
        uintptr_t modelDataAddress = *(uintptr_t*)(modelsList + 4 * i);
        CharacterData* model = (CharacterData*)(modelDataAddress);

        char* modelName = (char*)model->namePtr;

        std::string modelNameStr = std::string(modelName);
        std::string query = modelListQuery.c_str();
        std::transform(modelNameStr.begin(), modelNameStr.end(), modelNameStr.begin(), ::tolower);
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);

        if ((query != "" && (modelNameStr.find(query) == std::string::npos)))
        {
            continue;
        }

        bool isLoaded = IsModelLoaded(i);

        ImGui::PushID(i);

        if (ImGui::TreeNode(modelName))
        {
            char* modelLabel = (char*)model->labelPtr;
            const char* displayLabel = (std::string(modelLabel).find("TEXT STRING ERROR") != std::string::npos) ? "NO LABEL" : modelLabel;


            ImGui::Text("Address: %x", model);
            ImGui::Text("Label: %s", displayLabel);
            ImGui::Text("Path: %s", (char*)model->pathPtr);
            ImGui::Text("Index: %i", model->skinIndex);
            ImGui::Text("Loaded: %s", isLoaded ? "Yes" : "No");
            ImGui::Text("Char def file address: %x", model->charDefFile);

            ImGui::TreePop();
        }

        ImGui::SameLine();

        if (ImGui::Button("Swap to"))
        {
            if (!isLoaded)
            {
                Logs::Push("Loading %s ...\n", modelName);
                loadModelByIndex(harryGameAddress, i, 735);
                while (!IsModelLoaded(i));
                Logs::Push("Successfully loaded %s.\n", modelName);
            }

            SetPlayerModelIndex(i, modelName);
        }

        ImGui::PopID();
    }

    ImGui::End();
}