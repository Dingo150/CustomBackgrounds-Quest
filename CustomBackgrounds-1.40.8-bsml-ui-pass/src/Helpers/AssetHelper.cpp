#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "main.hpp"
#include "EmbeddedContent.hpp"

#include "UnityEngine/AssetBundle.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Object.hpp"
#include "System/Convert.hpp"
#include "System/Type.hpp"

UnityEngine::Material* LoadSkyMaterial()
{
    // Decode the embedded Base64 asset bundle
    auto bytes = System::Convert::FromBase64String(
        il2cpp_utils::newcsstr(FILE_SKYASSET)
    );

    static auto ab_LoadFromMemory =
        reinterpret_cast<function_ptr_t<UnityEngine::AssetBundle*, Array<uint8_t>*, uint>>(
            il2cpp_functions::resolve_icall(
                "UnityEngine.AssetBundle::LoadFromMemory_Internal"
            )
        );

    UnityEngine::AssetBundle* bundle = ab_LoadFromMemory(static_cast<Array<uint8_t>*>(bytes), 0);

    if (!bundle) return nullptr;

    // Load the material from the asset bundle
    static auto ab_LoadAllAssets =
        reinterpret_cast<function_ptr_t<Array<UnityEngine::Object*>*, UnityEngine::AssetBundle*, Il2CppString*, System::Type*>>(
            il2cpp_functions::resolve_icall(
                "UnityEngine.AssetBundle::LoadAssetWithSubAssets_Internal"
            )
        );

    auto materialArray = ab_LoadAllAssets(
        bundle,
        il2cpp_utils::newcsstr(""),
        csTypeOf(UnityEngine::Material*)
    );

        if (!materialArray || materialArray->get_Length() < 1) {
        bundle->Unload(false);
        return nullptr;
    }

    UnityEngine::Material* clonedMaterial =
        UnityEngine::Material::New_ctor(
            reinterpret_cast<UnityEngine::Material*>(materialArray->_values[0])
        );

    bundle->Unload(false);

    return clonedMaterial;
}