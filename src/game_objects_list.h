#pragma once
void RenderGameObjectsList();
extern bool showGameObjectsList;
extern float gameObjectsAlpha;

GameObject* GetPlayerGameObject();
std::vector<GameObject*> GetAllGameObjects();