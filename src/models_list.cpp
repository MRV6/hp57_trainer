#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_stdlib.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <format>
#include <thread>

#include "main.h"
#include "logs.h"

bool showModelsList = false;
std::string modelListQuery;
bool isSwappingModel = false;

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

static void SwapToModel(bool isLoaded, int modelIndex, const char* modelName)
{
    bool failedLoadingModel = false;
    if (!isLoaded)
    {
        const clock_t begin_time = clock();

        Logs::Push("Loading %s ...\n", modelName);

        // NOTE: Loading doesn't work in missions yet as the game is blocking this via a check in this function
        // HarryGame + 0x214C = in freemode
        // Need to patch this but atm idc
        int success = loadModelByIndex(harryGameAddress, modelIndex, 735); // TODO: Use the old model index as third arg

        while (!IsModelLoaded(modelIndex))
        {
            Logs::Push("Waiting for %s to load ...\n", modelName);

            if (float(clock() - begin_time) > 3000.0)
            {
                Logs::Push("Failed loading %s.\n", modelName);
                isSwappingModel = false;
                failedLoadingModel = true;
                break;
            }
        }

        if (!failedLoadingModel)
        {
            Logs::Push("Successfully loaded %s.\n", modelName);
        }
    }

    if (!failedLoadingModel)
    {
        SetPlayerModelIndex(modelIndex, modelName);
    }

    isSwappingModel = false;
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
            if (!isSwappingModel)
            {
                isSwappingModel = true;
                std::thread t1(SwapToModel, isLoaded, i, modelName);
                t1.detach();
            }
        }

        ImGui::PopID();
    }

    ImGui::End();
}