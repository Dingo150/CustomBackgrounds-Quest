
#include "main.hpp"
#include "EnvironmentHider.hpp"

#include "UnityEngine/Object.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/WaitForSeconds.hpp"

#include "GlobalNamespace/LightManager.hpp"
#include "GlobalNamespace/MultiplayerLobbyAvatarPlace.hpp"
#include "GlobalNamespace/MenuEnvironmentManager.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/utils/typedefs-string.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"

#include "custom-types/shared/coroutine.hpp"
#include "System/Collections/IEnumerator.hpp"

#include <regex>
#include <list>
#include <string>

using namespace UnityEngine;

bool getBGActive()
{
    return backgroundObject && backgroundObject->get_activeSelf();
}

UnityEngine::GameObject* FindMultiplayerPlatform()
{
    auto platforms =
        Resources::FindObjectsOfTypeAll<GlobalNamespace::MultiplayerLobbyAvatarPlace*>();

    if (!platforms)
        return nullptr;

    for (size_t i = 0; i < platforms.size(); i++)
    {
        auto platform = platforms[i];
        if (!platform)
            continue;

        auto plat = platform->get_transform();
        if (plat && plat->get_parent())
            return plat->get_parent()->get_gameObject();
    }

    return nullptr;
}

void HideChildRenderers(
    GameObject* obj,
    bool onlymeshes,
    bool unhide = false,
    bool ignorelayer = false
)
{
    if (!obj)
        return;

    bool enabled = !getBGActive() || unhide;

    auto rendarray = obj->GetComponentsInChildren<Renderer*>();

    if (!rendarray)
        return;

    for (size_t i = 0; i < rendarray.size(); i++)
    {
        Renderer* renderer = rendarray[i];

        if (!renderer)
            continue;

        if (onlymeshes && !renderer->GetComponent<MeshRenderer*>())
            continue;

        auto rendererObject = renderer->get_gameObject();

        if (!ignorelayer && rendererObject && rendererObject->get_layer() == 13)
            continue;

        renderer->set_enabled(enabled);
    }
}

custom_types::Helpers::Coroutine HideChildLights(
    GameObject* obj,
    bool unhide = false
)
{
    if (!obj)
        co_return;

    co_yield reinterpret_cast<System::Collections::IEnumerator*>(
        CRASH_UNLESS(WaitForSeconds::New_ctor(0.5f))
    );

    auto rendarray = obj->GetComponentsInChildren<Renderer*>();

    if (!rendarray)
        co_return;

    for (size_t i = 0; i < rendarray.size(); i++)
    {
        Renderer* renderer = rendarray[i];

        if (!renderer)
            continue;

        if (renderer->GetComponent<GlobalNamespace::LightManager*>())
            continue;

        auto rendererObject = renderer->get_gameObject();

        if (!rendererObject)
            continue;

        std::string rendererName =
            static_cast<std::string>(renderer->get_name());

        if (!std::regex_search(
                rendererName,
                std::regex("bloom|light", std::regex::icase)
            )
            && rendererObject->get_layer() != 13)
        {
            continue;
        }

        renderer->set_enabled(unhide);
    }

    co_return;
}

void HideMenuEnv()
{
    auto managers =
        Resources::FindObjectsOfTypeAll<GlobalNamespace::MenuEnvironmentManager*>();

    if (!managers || managers.size() == 0 || !managers[0])
        return;

    bool ismulti =
    static_cast<int>(managers[0]->_prevMenuEnvironmentType) == 2;

    bool bgActive = getBGActive();

    if (!ismulti)
    {
        auto floorObj = GameObject::Find(
            StringW("MenuEnvironmentManager/DefaultMenuEnvironment/BasicMenuGround")
        );

        auto notesObj = GameObject::Find(
            StringW("MenuEnvironmentManager/DefaultMenuEnvironment/Notes")
        );

        auto notePileObj = GameObject::Find(
            StringW("MenuEnvironmentManager/DefaultMenuEnvironment/PileOfNotes")
        );

        auto multiEnvObj = FindMultiplayerPlatform();

        if (multiEnvObj)
            multiEnvObj->SetActive(bgActive);

        HideChildRenderers(notesObj, false);
        HideChildRenderers(notePileObj, false);

        if (floorObj)
        {
            auto floorRenderer = floorObj->GetComponent<MeshRenderer*>();

            if (floorRenderer)
                floorRenderer->set_enabled(!bgActive);
        }

        if (bgActive && multiEnvObj)
        {
            HideChildRenderers(multiEnvObj, false, false, true);

            auto lobbyAvatarPlace = multiEnvObj->Find(
                StringW("LobbyAvatarPlace")
            );

            if (lobbyAvatarPlace)
                HideChildRenderers(lobbyAvatarPlace->get_gameObject(), false, true);
        }
    }
}

void HideGameEnv()
{
    GameObject* environmentObj =
        GameObject::Find(StringW("Environment"));

    GameObject* playersPlaceObj =
        GameObject::Find(StringW("Environment/PlayersPlace"));

    GameObject* coreLightingObj =
        GameObject::Find(StringW("Environment/CoreLighting"));

    if (getConfig().config["hideLasers"].GetBool() && environmentObj)
    {
        auto behaviour = environmentObj->GetComponent<MonoBehaviour*>();

        if (behaviour)
        {
            auto coroutine =
                custom_types::Helpers::CoroutineHelper::New(
                    HideChildLights(environmentObj)
                );

            behaviour->StartCoroutine(coroutine);
        }
    }

    if (getConfig().config["hideEnvironment"].GetBool())
    {
        HideChildRenderers(environmentObj, false);
        HideChildRenderers(playersPlaceObj, true, true);
    }

    HideChildRenderers(coreLightingObj, true, true);
}
