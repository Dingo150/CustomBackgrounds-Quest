
#include "BGConfig.hpp"
#include "EnvironmentHider.hpp"
#include "Helpers/AssetHelper.hpp"
#include "UI/BSMLSettings.hpp"
#include "scotland2/shared/loader.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/utils/typedefs-string.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "paper2_scotland2/shared/logger.hpp"
#include "BSML.hpp"

#include <sys/stat.h>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdarg>

#include "UnityEngine/Color.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/Component.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/TextureFormat.hpp"
#include "UnityEngine/ImageConversion.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"

#include "GlobalNamespace/MultiplayerLobbyAvatarPlace.hpp"
#include "GlobalNamespace/MenuEnvironmentManager.hpp"
#include "GlobalNamespace/TrackLaneRing.hpp"
#include "GlobalNamespace/Spectrogram.hpp"
#include "GlobalNamespace/MainCamera.hpp"

using namespace GlobalNamespace;

UnityEngine::GameObject* backgroundObject;
UnityEngine::Material* backgroundMat;
UnityEngine::Texture2D* backgroundTexture;
long mainOriginalCullMask = 0;

static modloader::ModInfo modInfo{"CustomBackgrounds", "1.3.2", 0};

std::string bgDirectoryPath;

// Keep the legacy printf-style call sites while routing logs through Paper2.
struct CompatLogger {
    template<typename... Args>
    void info(const char* f, Args... a) {
        char b[2048];
        if constexpr (sizeof...(Args) == 0)
            std::snprintf(b, sizeof(b), "%s", f);
        else
            std::snprintf(b, sizeof(b), f, a...);
        Paper::Logger::info("{}", b);
    }

    template<typename... Args>
    void debug(const char* f, Args... a) {
        char b[2048];
        if constexpr (sizeof...(Args) == 0)
            std::snprintf(b, sizeof(b), "%s", f);
        else
            std::snprintf(b, sizeof(b), f, a...);
        Paper::Logger::debug("{}", b);
    }

    template<typename... Args>
    void warn(const char* f, Args... a) {
        char b[2048];
        if constexpr (sizeof...(Args) == 0)
            std::snprintf(b, sizeof(b), "%s", f);
        else
            std::snprintf(b, sizeof(b), f, a...);
        Paper::Logger::warn("{}", b);
    }

    template<typename... Args>
    void warning(const char* f, Args... a) {
        warn(f, a...);
    }

    template<typename... Args>
    void error(const char* f, Args... a) {
        char b[2048];
        if constexpr (sizeof...(Args) == 0)
            std::snprintf(b, sizeof(b), "%s", f);
        else
            std::snprintf(b, sizeof(b), f, a...);
        Paper::Logger::error("{}", b);
    }

    template<typename... Args>
    void critical(const char* f, Args... a) {
        char b[2048];
        if constexpr (sizeof...(Args) == 0)
            std::snprintf(b, sizeof(b), "%s", f);
        else
            std::snprintf(b, sizeof(b), f, a...);
        Paper::Logger::critical("{}", b);
    }
};

CompatLogger& getLogger() {
    static CompatLogger logger;
    return logger;
}

Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

void CreateBGObject()
{
    if (!backgroundObject)
    {
        backgroundObject = UnityEngine::GameObject::CreatePrimitive(UnityEngine::PrimitiveType::Sphere);
        backgroundObject->set_name(il2cpp_utils::newcsstr("_CustomBackground"));
        backgroundObject->set_layer(29);
        UnityEngine::Object::DontDestroyOnLoad(backgroundObject);

        UnityEngine::Renderer* bgrenderer = backgroundObject->GetComponent<UnityEngine::Renderer*>();
        if (!bgrenderer)
        {
            UnityEngine::Object::Destroy(backgroundObject);
            backgroundObject = nullptr;
            getLogger().error("[CustomBackgrounds] Could not create the background renderer.");
            return;
        }

        UnityEngine::Material* skyMaterial = LoadSkyMaterial();
        if (!skyMaterial)
        {
            UnityEngine::Object::Destroy(backgroundObject);
            backgroundObject = nullptr;
            getLogger().error("[CustomBackgrounds] Could not load the embedded sky material.");
            return;
        }

        bgrenderer->set_sortingOrder(-8192);
        bgrenderer->set_material(skyMaterial);
        backgroundMat = bgrenderer->get_material();

        if (backgroundMat && backgroundTexture)
            backgroundMat->SetTexture(il2cpp_utils::newcsstr("_MainTex"), backgroundTexture);

        UnityEngine::Transform* bgtrans = backgroundObject->get_transform();
        bgtrans->set_localScale(UnityEngine::Vector3(-800.0f, -800.0f, -800.0f));
        bgtrans->set_localPosition(UnityEngine::Vector3::get_zero());
        bgtrans->set_localEulerAngles(UnityEngine::Vector3(
            0.0f,
            static_cast<float>(getConfig().config["rotationOffset"].GetInt() - 90),
            180.0f
        ));
    }
}

void LoadBackground(std::string path)
{
    const auto separator = path.find_last_of("/\\");
    const std::string filename = separator == std::string::npos ? path : path.substr(separator + 1);

    if (!fileexists(path))
    {
        getLogger().warning("[CustomBackgrounds] Background file not found: %s", filename.c_str());
        return;
    }

    std::ifstream instream(path, std::ios::in | std::ios::binary);
    if (!instream.is_open())
    {
        getLogger().error("[CustomBackgrounds] Could not open background file: %s", filename.c_str());
        return;
    }

    std::vector<uint8_t> data(
        (std::istreambuf_iterator<char>(instream)),
        std::istreambuf_iterator<char>()
    );
    instream.close();

    if (data.empty())
    {
        getLogger().error("[CustomBackgrounds] Background file is empty: %s", filename.c_str());
        return;
    }

    auto bytes = il2cpp_utils::vectorToArray(data);
    if (!bytes)
    {
        getLogger().error("[CustomBackgrounds] Could not allocate image data for: %s", filename.c_str());
        return;
    }

    const int width = getConfig().config["imageWidth"].GetInt();
    const int height = std::max(
        1,
        static_cast<int>(std::floor(static_cast<float>(width) * (2.0f / 3.0f)))
    );

    UnityEngine::Texture2D* candidateTexture =
        UnityEngine::Texture2D::New_ctor(
            width,
            height,
            UnityEngine::TextureFormat::RGBA32,
            false,
            false
        );

    if (!candidateTexture)
    {
        getLogger().error("[CustomBackgrounds] Could not create a texture for: %s", filename.c_str());
        return;
    }

    const bool success = UnityEngine::ImageConversion::LoadImage(candidateTexture, bytes, false);

    if (!success)
    {
        UnityEngine::Object::Destroy(candidateTexture);
        getLogger().error("[CustomBackgrounds] Unsupported or invalid image: %s", filename.c_str());
        return;
    }

    if (backgroundTexture)
        UnityEngine::Object::Destroy(backgroundTexture);

    backgroundTexture = candidateTexture;

    getLogger().info("[CustomBackgrounds] Successfully loaded '%s'.", filename.c_str());

    if (backgroundMat)
        backgroundMat->SetTexture(il2cpp_utils::newcsstr("_MainTex"), backgroundTexture);
    else
        CreateBGObject();
}

void InitBackgrounds()
{
    auto& modcfg = getConfig().config;
    std::string backgroundPath = bgDirectoryPath;
    backgroundPath += modcfg["selectedFile"].GetString();

    if (fileexists(backgroundPath))
        LoadBackground(backgroundPath);
}

MAKE_HOOK_MATCH(
    SceneManager_SceneChanged,
    &UnityEngine::SceneManagement::SceneManager::Internal_ActiveSceneChanged,
    void,
    UnityEngine::SceneManagement::Scene previousScene,
    UnityEngine::SceneManagement::Scene nextScene
)
{
    SceneManager_SceneChanged(previousScene, nextScene);

    std::string nextSceneName = static_cast<std::string>(nextScene.get_name());
    auto& modcfg = getConfig().config;

    if ((nextSceneName == "HealthWarning" || nextSceneName == "MenuViewControllers")
        && !backgroundObject
        && modcfg["enabled"].GetBool())
    {
        InitBackgrounds();
        CreateBGObject();
    }
}

MAKE_HOOK_MATCH(
    TrackLaneRing_Init,
    &GlobalNamespace::TrackLaneRing::Init,
    void,
    GlobalNamespace::TrackLaneRing* instance,
    UnityEngine::Vector3 pos,
    UnityEngine::Vector3 offset
)
{
    TrackLaneRing_Init(instance, pos, offset);

    bool hide = getConfig().config["hideRings"].GetBool()
        && getConfig().config["enabled"].GetBool();

    auto renderers = instance->GetComponentsInChildren<UnityEngine::Renderer*>();

    for (size_t i = 0; i < renderers.size(); i++)
    {
        UnityEngine::Renderer* rend = renderers[i];
        if (!rend) continue;

        rend->get_gameObject()->set_layer(hide ? 14 : 0);
        rend->set_enabled(!hide);
    }
}

MAKE_HOOK_MATCH(
    Spectrogram_Awake,
    &GlobalNamespace::Spectrogram::Awake,
    void,
    GlobalNamespace::Spectrogram* instance
)
{
    Spectrogram_Awake(instance);

    if (backgroundObject && getConfig().config["enabled"].GetBool())
        HideGameEnv();
}

MAKE_HOOK_MATCH(
    MainCamera_Awake,
    &GlobalNamespace::MainCamera::Awake,
    void,
    GlobalNamespace::MainCamera* caminstance
)
{
    MainCamera_Awake(caminstance);

    auto& modcfg = getConfig().config;
    Il2CppString* sceneName =
        UnityEngine::SceneManagement::SceneManager::GetActiveScene().get_name();

    UnityEngine::Camera* maincam = caminstance->camera;

    if (maincam && sceneName == il2cpp_utils::newcsstr("GameCore") && modcfg["enabled"].GetBool())
    {
        mainOriginalCullMask =
            (mainOriginalCullMask == 0) ? maincam->get_cullingMask() : mainOriginalCullMask;

        if (modcfg["hideEnvironment"].GetBool())
            maincam->set_cullingMask(maincam->get_cullingMask() & ~(1 << 14));

        if (modcfg["hideLasers"].GetBool())
            maincam->set_cullingMask(maincam->get_cullingMask() & ~(1 << 13));
    }
}

MAKE_HOOK_MATCH(
    MenuEnvManager_ShowEnv,
    &GlobalNamespace::MenuEnvironmentManager::ShowEnvironmentType,
    void,
    GlobalNamespace::MenuEnvironmentManager* instance,
    GlobalNamespace::MenuEnvironmentManager::MenuEnvironmentType envtype
)
{
    MenuEnvManager_ShowEnv(instance, envtype);

    if (backgroundObject)
        HideMenuEnv();
}

extern "C" void setup(CModInfo* info) noexcept
{
    Paper::Logger::info("[CustomBackgrounds] SETUP REACHED");

    if (!info)
    {
        Paper::Logger::error("[CustomBackgrounds] SETUP FAILED: info is null");
        return;
    }

    Paper::Logger::info("[CustomBackgrounds] SETUP: info is valid");

    modInfo = modloader::ModInfo(*info);
    Paper::Logger::info("[CustomBackgrounds] SETUP: modInfo created");

    bgDirectoryPath = "/sdcard/ModData/com.beatgames.beatsaber/Mods/CustomBackgrounds/";
    Paper::Logger::info("[CustomBackgrounds] DATA DIRECTORY: {}", bgDirectoryPath);

    struct stat directoryInfo{};

if (stat(bgDirectoryPath.c_str(), &directoryInfo) == 0 && S_ISDIR(directoryInfo.st_mode)) {
    Paper::Logger::info("[CustomBackgrounds] Background directory already exists");
}
else if (mkdir(bgDirectoryPath.c_str(), 0777) == 0) {
    Paper::Logger::info("[CustomBackgrounds] Background directory created successfully");
}
else {
    Paper::Logger::info("[CustomBackgrounds] ERROR: Could not create background directory");
}

    getConfig().Load();
    Paper::Logger::info("[CustomBackgrounds] SETUP COMPLETE");
}

extern "C" void late_load() noexcept
{
    Paper::Logger::info("[CustomBackgrounds] LOAD REACHED");

    if (!LoadConfig())
    {
        Paper::Logger::info("[CustomBackgrounds] Creating config");
        SetupConfig();
    }

    Paper::Logger::info("[CustomBackgrounds] Initialising IL2CPP");
    il2cpp_functions::Init();

    Paper::Logger::info("[CustomBackgrounds] Initialising BSML");
    BSML::Init();

    Paper::Logger::info("[CustomBackgrounds] Installing hooks");
    INSTALL_HOOK(getLogger(), SceneManager_SceneChanged);
    INSTALL_HOOK(getLogger(), MenuEnvManager_ShowEnv);
    INSTALL_HOOK(getLogger(), TrackLaneRing_Init);
    INSTALL_HOOK(getLogger(), Spectrogram_Awake);
    INSTALL_HOOK(getLogger(), MainCamera_Awake);

    Paper::Logger::info("[CustomBackgrounds] Registering menu");

    BSML::Register::RegisterMainMenuViewControllerMethod(
        "Custom Backgrounds",
        "Backgrounds",
        "Configure custom backgrounds",
        CustomBackgrounds::BSMLSettings::Build
    );

    Paper::Logger::info("[CustomBackgrounds] LOAD COMPLETE");
}



