
#include "main.hpp"
#include "BSML.hpp"
#include "BSML-Lite/Creation/Settings.hpp"
#include "BSML-Lite/Creation/Buttons.hpp"
#include "BSML-Lite/Creation/Text.hpp"
#include "BSML-Lite/TransformWrapper.hpp"

#include "BSML/Components/Settings/ToggleSetting.hpp"
#include "BSML/Components/Settings/DropdownListSetting.hpp"
#include "BSML/Components/Settings/IncrementSetting.hpp"

#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs-string.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string_view>

namespace CustomBackgrounds::BSMLSettings {

    static void AdjustIncrement(BSML::IncrementSetting* setting) {
        if (!setting || !setting->text ||
            !setting->decButton || !setting->incButton)
            return;

        auto label = setting->text->get_transform();
        auto dec = setting->decButton->get_transform();
        auto inc = setting->incButton->get_transform();

        if (!label || !dec || !inc)
            return;

        auto labelPos = label->get_localPosition();
        auto decPos = dec->get_localPosition();
        auto incPos = inc->get_localPosition();

        float alignedY = (labelPos.y + decPos.y + incPos.y) / 3.0f;

        labelPos.x += 25.0f;
        decPos.x -= 25.0f;
        incPos.x -= 25.0f;

        labelPos.y = alignedY;
        decPos.y = alignedY;
        incPos.y = alignedY;

        label->set_localPosition(labelPos);
        dec->set_localPosition(decPos);
        inc->set_localPosition(incPos);
    }

    static void SaveBool(char const* key, bool value) {
        auto& cfg = getConfig();
        cfg.config[key].SetBool(value);
        cfg.Write();
    }

    static void SaveInt(char const* key, int value) {
        auto& cfg = getConfig();
        cfg.config[key].SetInt(value);
        cfg.Write();
    }

    static void OnEnabled(bool value) {
        SaveBool("enabled", value);

        if (backgroundObject)
            backgroundObject->SetActive(value);

        if (value && !backgroundObject)
            LoadBackground(bgDirectoryPath + getConfig().config["selectedFile"].GetString());
    }

    static void OnHideEnvironment(bool value) {
        SaveBool("hideEnvironment", value);
    }

    static void OnHideRings(bool value) {
        SaveBool("hideRings", value);
    }

    static void OnHideLights(bool value) {
        SaveBool("hideLasers", value);
    }

    static void OnRotation(float value) {
        int rotation = ((static_cast<int>(value) % 360) + 360) % 360;
        SaveInt("rotationOffset", rotation);

        if (backgroundObject)
            backgroundObject->get_transform()->set_localEulerAngles(
                UnityEngine::Vector3(0, rotation - 90, 180)
            );
    }

    static void OnResolution(float value) {
        int width = std::clamp(static_cast<int>(value), 3840, 11520);
        SaveInt("imageWidth", width);
    }

    static std::vector<std::string> imageNames;
    static std::vector<std::string_view> imageViews;

    static void RefreshImages() {
        imageNames.clear();
        imageViews.clear();

        std::error_code ec;

        for (auto const& entry : std::filesystem::directory_iterator(bgDirectoryPath, ec)) {
            if (ec || !entry.is_regular_file())
                continue;

            auto ext = entry.path().extension().string();

            std::transform(
                ext.begin(),
                ext.end(),
                ext.begin(),
                [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                }
            );

            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                imageNames.push_back(entry.path().filename().string());
        }

        std::sort(imageNames.begin(), imageNames.end());

        for (auto const& name : imageNames)
            imageViews.emplace_back(name);
    }

    static void OnReloadBackground() {
        const std::string name =
            getConfig().config["selectedFile"].GetString();

        if (!name.empty())
            LoadBackground(bgDirectoryPath + name);
    }

    static void OnImageSelected(StringW selected) {
        std::string name = static_cast<std::string>(selected);

        if (name.empty())
            return;

        auto& cfg = getConfig();

        cfg.config["selectedFile"].SetString(
            name.c_str(),
            cfg.config.GetAllocator()
        );

        cfg.Write();
        LoadBackground(bgDirectoryPath + name);
    }

    void Build(
        HMUI::ViewController* viewController,
        bool firstActivation,
        bool addedToHierarchy,
        bool screenSystemEnabling
    ) {
        if (!viewController || !firstActivation || !addedToHierarchy)
            return;

        auto parent = viewController->get_transform();
        auto& cfg = getConfig().config;

        // Existing toggles - unchanged.

        BSML::Lite::CreateToggle(
            parent, "Enable Custom Backgrounds",
            cfg["enabled"].GetBool(),
            UnityEngine::Vector2(0, -4), OnEnabled
        );

        BSML::Lite::CreateToggle(
            parent, "Hide Environment",
            cfg["hideEnvironment"].GetBool(),
            UnityEngine::Vector2(0, -11), OnHideEnvironment
        );

        BSML::Lite::CreateToggle(
            parent, "Hide Rings",
            cfg["hideRings"].GetBool(),
            UnityEngine::Vector2(0, -18), OnHideRings
        );

        BSML::Lite::CreateToggle(
            parent, "Hide Lights",
            cfg["hideLasers"].GetBool(),
            UnityEngine::Vector2(0, -25), OnHideLights
        );

        // Background image selection.

        RefreshImages();

        if (!imageViews.empty()) {
            std::string selected = cfg["selectedFile"].GetString();

            auto dropdownSetting = BSML::Lite::CreateDropdown(
                parent,
                "Background Image",
                il2cpp_utils::newcsstr(selected),
                imageViews,
                OnImageSelected
            );

            if (dropdownSetting) {
                auto labelTransform = dropdownSetting->get_transform();

                if (labelTransform) {
                    labelTransform->set_localPosition(
                        UnityEngine::Vector3(25, -32, 0)
                    );
                }

                if (dropdownSetting->dropdown) {
                    auto dropdownTransform = dropdownSetting->dropdown->get_transform();

                    if (dropdownTransform) {
                        auto pos = dropdownTransform->get_localPosition();
                        pos.x += 25.0f;
                        pos.y = -32.0f;
                        dropdownTransform->set_localPosition(pos);
                    }
                }
            }
        }
        else {
            BSML::Lite::CreateText(
                parent,
                il2cpp_utils::newcsstr(
                    "No background images found.\n"
                    "Please add PNG, JPG or JPEG files to your "
                    "CustomBackgrounds folder."
                ),
                TMPro::FontStyles::Normal,
                3.0f,
                UnityEngine::Vector2(25, -32),
                UnityEngine::Vector2(100, 12)
            );
        }

        // Rotation setting - unchanged.

        auto rotationSetting = BSML::Lite::CreateIncrementSetting(
            parent,
            "Rotation Offset",
            0,
            15.0f,
            static_cast<float>(cfg["rotationOffset"].GetInt()),
            0.0f,
            359.0f,
            UnityEngine::Vector2(0, -46),
            OnRotation
        );

        AdjustIncrement(rotationSetting);

        // Resolution setting - unchanged.

        auto resolutionSetting = BSML::Lite::CreateIncrementSetting(
            parent,
            "Target Resolution",
            0,
            3840.0f,
            static_cast<float>(cfg["imageWidth"].GetInt()),
            3840.0f,
            11520.0f,
            UnityEngine::Vector2(0, -53),
            OnResolution
        );

        AdjustIncrement(resolutionSetting);
    }
}
