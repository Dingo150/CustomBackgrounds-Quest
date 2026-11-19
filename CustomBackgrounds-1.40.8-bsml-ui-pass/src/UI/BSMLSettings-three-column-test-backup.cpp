#include "main.hpp"
#include "BSML.hpp"
#include "BSML-Lite/Creation/Settings.hpp"
#include "BSML-Lite/Creation/Buttons.hpp"
#include "BSML-Lite/Creation/Text.hpp"
#include "BSML-Lite/TransformWrapper.hpp"

#include "BSML/Components/Settings/ToggleSetting.hpp"
#include "BSML/Components/Settings/DropdownListSetting.hpp"
#include "BSML/Components/Settings/SliderSetting.hpp"

#include "HMUI/RangeValuesTextSlider.hpp"
#include "System/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs-string.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <cctype>

namespace CustomBackgrounds::BSMLSettings {

    static BSML::DropdownListSetting* imageDropdown = nullptr;
    static UnityEngine::GameObject* emptyMessageObject = nullptr;
    static UnityEngine::Transform* settingsParent = nullptr;

    static std::vector<std::string> imageNames;
    static std::vector<std::string_view> imageViews;

    // Column positions shared by the different controls.
    static constexpr float labelX = -40.0f;
    static constexpr float controlX = 0.0f;
    static constexpr float rightX = 42.0f;

    static void SetLocalX(UnityEngine::Transform* transform, float x) {
        if (!transform)
            return;

        auto position = transform->get_localPosition();
        position.x = x;
        transform->set_localPosition(position);
    }

    static void AdjustToggle(BSML::ToggleSetting* setting) {
        if (!setting || !setting->text || !setting->toggle)
            return;

        SetLocalX(setting->text->get_transform(), labelX);
        SetLocalX(setting->toggle->get_transform(), rightX);
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

    static void SaveSelectedFile(std::string const& name) {
        auto& cfg = getConfig();

        cfg.config["selectedFile"].SetString(
            name.c_str(),
            cfg.config.GetAllocator()
        );

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

    static void RefreshImages() {
        imageNames.clear();
        imageViews.clear();

        std::error_code ec;

        for (auto const& entry :
             std::filesystem::directory_iterator(bgDirectoryPath, ec)) {

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

    static void OnImageSelected(StringW selected) {
        std::string name = static_cast<std::string>(selected);

        if (name.empty())
            return;

        SaveSelectedFile(name);
        LoadBackground(bgDirectoryPath + name);
    }

    static void SetDropdownActive(bool active) {
        if (!imageDropdown)
            return;

        auto transform = imageDropdown->get_transform();

        if (transform) {
            auto gameObject = transform->get_gameObject();

            if (gameObject)
                gameObject->SetActive(active);
        }
    }

    static void SetEmptyMessageActive(bool active) {
        if (emptyMessageObject)
            emptyMessageObject->SetActive(active);
    }

    static void CreateImageDropdown() {
        if (!settingsParent || imageViews.empty())
            return;

        std::string selected =
            getConfig().config["selectedFile"].GetString();

        auto selectedIt = std::find(
            imageNames.begin(),
            imageNames.end(),
            selected
        );

        if (selectedIt == imageNames.end()) {
            selected = imageNames.front();
            SaveSelectedFile(selected);
        }

        imageDropdown = BSML::Lite::CreateDropdown(
            settingsParent,
            "Background Image",
            il2cpp_utils::newcsstr(selected),
            imageViews,
            OnImageSelected
        );

        if (!imageDropdown)
            return;


        if (imageDropdown->dropdown)
            SetLocalX(imageDropdown->dropdown->get_transform(), controlX);

        SetDropdownActive(true);
    }

    static void UpdateImageDropdown() {
        if (!imageDropdown || imageNames.empty())
            return;

        std::string selected =
            getConfig().config["selectedFile"].GetString();

        auto selectedIt = std::find(
            imageNames.begin(),
            imageNames.end(),
            selected
        );

        bool selectionChanged = selectedIt == imageNames.end();

        if (selectionChanged) {
            selected = imageNames.front();
            SaveSelectedFile(selected);
        }

        int selectedIndex = static_cast<int>(
            std::distance(
                imageNames.begin(),
                std::find(imageNames.begin(), imageNames.end(), selected)
            )
        );

        imageDropdown->values.clear();

        for (auto const& name : imageNames) {
            auto il2cppString =
                il2cpp_utils::newcsstr(std::string_view(name));

            imageDropdown->values.push_back(
                reinterpret_cast<System::Object*>(il2cppString)
            );
        }

        imageDropdown->index = selectedIndex;
        imageDropdown->UpdateChoices();

        if (selectionChanged)
            LoadBackground(bgDirectoryPath + selected);
    }

    static void OnRefresh() {
        RefreshImages();

        if (imageNames.empty()) {
            SetDropdownActive(false);
            SetEmptyMessageActive(true);
            return;
        }

        SetEmptyMessageActive(false);

        if (!imageDropdown)
            CreateImageDropdown();
        else
            UpdateImageDropdown();

        SetDropdownActive(true);
    }

    static void AdjustSlider(BSML::SliderSetting* setting) {
        if (!setting || !setting->text || !setting->slider)
            return;

        SetLocalX(setting->text->get_transform(), labelX);
        SetLocalX(setting->slider->get_transform(), controlX);

        auto valueText =
            setting->slider->__cordl_internal_get__valueText();

        if (valueText)
            SetLocalX(valueText->get_transform(), rightX);
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
        settingsParent = parent;

        auto& cfg = getConfig().config;

        // LEFT: setting names. RIGHT: toggle switches.

        auto enabledSetting = BSML::Lite::CreateToggle(
            parent,
            "Enable Custom Backgrounds",
            cfg["enabled"].GetBool(),
            UnityEngine::Vector2(0, -4),
            OnEnabled
        );
        AdjustToggle(enabledSetting);

        auto environmentSetting = BSML::Lite::CreateToggle(
            parent,
            "Hide Environment",
            cfg["hideEnvironment"].GetBool(),
            UnityEngine::Vector2(0, -11),
            OnHideEnvironment
        );
        AdjustToggle(environmentSetting);

        auto ringsSetting = BSML::Lite::CreateToggle(
            parent,
            "Hide Rings",
            cfg["hideRings"].GetBool(),
            UnityEngine::Vector2(0, -18),
            OnHideRings
        );
        AdjustToggle(ringsSetting);

        auto lightsSetting = BSML::Lite::CreateToggle(
            parent,
            "Hide Lights",
            cfg["hideLasers"].GetBool(),
            UnityEngine::Vector2(0, -25),
            OnHideLights
        );
        AdjustToggle(lightsSetting);

        // Find available background images.

        RefreshImages();

        // Message shown when the folder contains no images.

        auto message = BSML::Lite::CreateText(
            parent,
            il2cpp_utils::newcsstr(
                "No background images found.\n"
                "Please add PNG, JPG or JPEG files to your "
                "CustomBackgrounds folder."
            ),
            TMPro::FontStyles::Normal,
            3.0f,
            UnityEngine::Vector2(0, -38),
            UnityEngine::Vector2(100, 12)
        );

        if (message) {
            auto messageTransform = message->get_transform();

            if (messageTransform)
                emptyMessageObject = messageTransform->get_gameObject();
        }

        // LEFT: Background Image label.
        // MIDDLE: image dropdown.
        // RIGHT: Refresh button.

        if (!imageNames.empty())
            CreateImageDropdown();

        SetEmptyMessageActive(imageNames.empty());

        BSML::Lite::CreateUIButton(
            parent,
            il2cpp_utils::newcsstr("Refresh"),
            "PracticeButton",
            UnityEngine::Vector2(rightX, -32),
            UnityEngine::Vector2(20, 5),
            OnRefresh
        );

        // LEFT: Rotation Offset label.
        // MIDDLE: slider.
        // RIGHT: displayed value.

        auto rotationSetting = BSML::Lite::CreateSliderSetting(
            parent,
            "Rotation Offset",
            15.0f,
            static_cast<float>(cfg["rotationOffset"].GetInt()),
            0.0f,
            359.0f,
            0.0f,
            false,
            UnityEngine::Vector2(0, -45),
            OnRotation
        );

        AdjustSlider(rotationSetting);

        // LEFT: Target Resolution label.
        // MIDDLE: slider.
        // RIGHT: displayed value.

        auto resolutionSetting = BSML::Lite::CreateSliderSetting(
            parent,
            "Target Resolution",
            3840.0f,
            static_cast<float>(cfg["imageWidth"].GetInt()),
            3840.0f,
            11520.0f,
            0.0f,
            false,
            UnityEngine::Vector2(0, -55),
            OnResolution
        );

        AdjustSlider(resolutionSetting);
    }
}

