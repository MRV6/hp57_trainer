#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_stdlib.h"
#include "main.h"
#include <string>
#include <algorithm>
#include <iostream>

bool showModelsList = false;
bool onlyLoadedModels = false;

std::string modelListQuery;

void SetPlayerEntityIndex(int index)
{
    uintptr_t harryGameObjectAddress = *(uintptr_t*)harryGameObjectPtr;
    if (harryGameObjectAddress)
    {
        std::cout << "Set player entity index to " << index << std::endl;
        int success = setPlayerEntityIndex(*(uintptr_t*)harryGameObjectAddress, index, 550, 1, false, true);
        //std::cout << "Set player entity index result " << success << std::cout;
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


        int unkModelValue = getUnkEntityValue(*(uintptr_t*)modelsClassAddress, i, 0xE);
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
    }

    ImGui::End();
}