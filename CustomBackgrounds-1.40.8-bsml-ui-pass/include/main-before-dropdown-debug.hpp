#pragma once
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/GameObject.hpp"
#include <string>

Configuration& getConfig();
extern UnityEngine::Texture2D* backgroundTexture;
extern UnityEngine::GameObject* backgroundObject;
extern std::string bgDirectoryPath;
extern void LoadBackground(std::string filename);
extern void CreateBGObject();
extern void InitBackgrounds();
