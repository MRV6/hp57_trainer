#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_stdlib.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <format>
#include "main.h"
#include "logs.h"

bool showModelsList = false;
bool onlyLoadedModels = false;

std::string modelListQuery;

void SetPlayerModelIndex(int index, const char* modelName)
{
    uintptr_t harryGameObjectAddress = *(uintptr_t*)harryGameObjectPtr;

    if (harryGameObjectAddress)
    {
        int success = setPlayerModelIndex(harryGameObjectAddress, index, 550, 1, false, true);

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

void RenderModelsList()
{
    ImGui::Begin("Models list");

    uintptr_t world = *(uintptr_t*)modelsClassAddress;
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


        int unkModelValue = getUnkModelValue(*(uintptr_t*)modelsClassAddress, i, 0xE);
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
                SetPlayerModelIndex(i, modelName);
            }
            ImGui::PopID();
        }
    }

    ImGui::End();
}