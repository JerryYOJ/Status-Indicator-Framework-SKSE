// SIF_API.h - other plugins #include this
#pragma once

#include <cstdint>
#include <functional>
#include <json/json.h>
#include <Windows.h>

namespace SIF {
    // Base class other plugins inherit from to create custom conditions
    class ICondition {
    public:
        virtual ~ICondition() = default;
        virtual bool Match(RE::TESObjectREFR* ref) const = 0;
    };

    // Builder signature: takes JSON value, returns a condition (or nullptr)
    // The RE::FormType argument is reserved as optional rule context.
    using ConditionBuilder = std::function<
        std::unique_ptr<ICondition>(const Json::Value& value, RE::FormType type)
    >;

    inline HMODULE GetCurrentModule()
    {
        HMODULE module = nullptr;
        GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
            reinterpret_cast<LPCTSTR>(&GetCurrentModule),
            &module);

        return module;
    }

    // The API interface - stable ABI via pure virtual
    class IAPI {
    public:
        virtual ~IAPI() = default;

        // Register a custom match field builder
        // name = JSON key (e.g. "myModCondition")
        // builder = factory that creates the condition from JSON
        virtual bool RegisterCondition(
            const char* name,
            ConditionBuilder builder
        ) = 0;

        // These wrappers automatically identify the calling plugin by resolving
        // the module that contains this inline function. Plugins should use these
        // _public methods instead of the internal virtual interface.

        inline void RegisterTrackedRef_public(RE::TESObjectREFR* ref) {
            RegisterTrackedRef(GetCurrentModule(), ref);
        }
        inline void UnregisterTrackedRef_public(RE::TESObjectREFR* ref) {
            UnregisterTrackedRef(GetCurrentModule(), ref);
        }
        inline void ClearTrackedRef_public() {
            ClearTrackedRef(GetCurrentModule());
        }
    private:
        virtual void RegisterTrackedRef(
            HMODULE key,
            RE::TESObjectREFR* ref
        ) = 0;

        virtual void UnregisterTrackedRef(
            HMODULE key,
            RE::TESObjectREFR* ref
        ) = 0;
        virtual void ClearTrackedRef(HMODULE key) = 0;
        // API version for compatibility checks
        virtual uint32_t GetVersion() const = 0;
    };

    // Message type for SKSE messaging
    constexpr uint32_t kMessage_GetAPI = 0xD111;

    // Example of how a plugin would register a condition builder
    // SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    //     SKSE::Init(skse);
    //     SIF::ListenForRegistration([](SKSE::MessagingInterface::Message* msg) {
    //         if (msg->type == SIF::kMessage_GetAPI) {
    //             auto* api = static_cast<SIF::IAPI*>(msg->data);
    //             api->RegisterCondition("myCustomField", [](const Json::Value& match, RE::FormType type)
    //                 -> std::unique_ptr<SIF::ICondition> {
    //                     // build and return your condition
    //                 });
    //         }
    //     });
    //     return true;
    // }
    inline void ListenForRegistration(SKSE::MessagingInterface::EventCallback* cb) {
        SKSE::GetMessagingInterface()->RegisterListener("StatusIndicatorFramework", cb);
    }
}
