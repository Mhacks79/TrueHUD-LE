#include "Runtime.h"
#include "Settings.h"
#include "SpecialResources.h"
#include <cstring>
#include "skse/PapyrusNativeFunctions.h"

namespace TrueHUDLE {
    namespace {
        volatile LONG callbackThread = 0;
        void LogCallbackThread() {
            const DWORD thread = GetCurrentThreadId();
            if (!InterlockedCompareExchange(&callbackThread, static_cast<LONG>(thread), 0))
                Log("Papyrus settings callback thread=%lu", thread);
        }
        float ReadGuarded(const char* id) {
            if (Disabled()) return 0;
            __try {
                LogCallbackThread();
                if (id && !strcmp(id, "__SpecialResourcesControlled"))
                    return ReadSpecialStatus().owner != InvalidResourceOwner ? 1.f : 0.f;
                return static_cast<float>(GetSetting(id));
            }
            __except(Fault("Papyrus ReadSetting", GetExceptionInformation())) { return 0; }
        }
        void WriteGuarded(const char* id, float value) {
            if (Disabled()) return;
            __try { LogCallbackThread(); SetSetting(id, value); }
            __except(Fault("Papyrus WriteSetting", GetExceptionInformation())) {}
        }
        float ReadSettingNative(TESQuest*, BSFixedString id) {
            return ReadGuarded(id.data);
        }
        void WriteSettingNative(TESQuest*, BSFixedString id, float value) {
            WriteGuarded(id.data, value);
        }
        void CommitSettingsNative(TESQuest*) {
            if (Disabled()) return;
            __try {
                LogCallbackThread();
                if (!WriteSettings()) Log("MCM settings could not be written; values remain active this session");
            }
            __except(Fault("Papyrus CommitSettings", GetExceptionInformation())) {}
        }
    }
    bool RegisterPapyrus(VMClassRegistry* registry) {
        registry->RegisterFunction(new NativeFunction1<TESQuest, float, BSFixedString>(
            "ReadSetting", "TrueHUD_MCM", ReadSettingNative, registry));
        registry->RegisterFunction(new NativeFunction2<TESQuest, void, BSFixedString, float>(
            "WriteSetting", "TrueHUD_MCM", WriteSettingNative, registry));
        registry->RegisterFunction(new NativeFunction0<TESQuest, void>(
            "CommitSettings", "TrueHUD_MCM", CommitSettingsNative, registry));
        return true;
    }
}
