#include "Runtime.h"
#include "HUDSignals.h"
#include "Settings.h"
#include "DamageSignals.h"
#include "LootNotifications.h"
#include "skse/skse_version.h"
#include <shlobj.h>
#include <cstdio>
#include <cstdarg>

namespace TrueHUDLE {
    namespace {
        FILE* logFile = nullptr;
        SRWLOCK logLock = SRWLOCK_INIT;
        volatile LONG faults = 0;
        DWORD loadThread = 0;
        _purecall_handler previousPurecall = nullptr;
    }
    void Log(const char* format, ...) {
        AcquireSRWLockExclusive(&logLock);
        if (logFile) {
            va_list args;
            va_start(args, format);
            vfprintf(logFile, format, args);
            va_end(args);
            fputc('\n', logFile);
            fflush(logFile);
        }
        ReleaseSRWLockExclusive(&logLock);
    }
    bool Disabled() { return InterlockedCompareExchange(&faults, 0, 0) >= 3; }
    static void LogPurecallBody() {
        void* frames[24];
        const USHORT count = CaptureStackBackTrace(0, 24, frames, nullptr);
        Log("PURECALL in TrueHUD CRT: thread=%lu frames=%u", GetCurrentThreadId(), unsigned(count));
        for (unsigned i = 0; i < count; ++i) {
            HMODULE module = nullptr; char path[MAX_PATH]{};
            if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                  reinterpret_cast<LPCSTR>(frames[i]), &module) &&
                GetModuleFileNameA(module, path, MAX_PATH)) {
                path[MAX_PATH-1] = 0;
                const char* leaf = strrchr(path, '\\');
                Log("PURECALL frame=%u module=%s RVA=%08X", i, leaf ? leaf+1 : path,
                    UInt32(reinterpret_cast<uintptr_t>(frames[i]) - reinterpret_cast<uintptr_t>(module)));
            } else Log("PURECALL frame=%u address=%p", i, frames[i]);
        }
    }
    static void OnPurecall() {
        __try { LogPurecallBody(); }
        __except(EXCEPTION_EXECUTE_HANDLER) {}
        if (previousPurecall && previousPurecall != OnPurecall) previousPurecall();
        // Return to the CRT's normal fatal handling; do not suppress the error.
    }
    int Fault(const char* context, EXCEPTION_POINTERS* exception) {
        const LONG count = InterlockedIncrement(&faults);
        const auto record = exception->ExceptionRecord;
        Log("FAULT %s code=%08lX at=%p access=%p count=%ld disabled=%u", context,
            record->ExceptionCode, record->ExceptionAddress,
            reinterpret_cast<void*>(record->NumberParameters > 1 ? record->ExceptionInformation[1] : 0),
            count, count >= 3);
        return EXCEPTION_EXECUTE_HANDLER;
    }
    static void OnMessageBody(SKSEMessagingInterface::Message* message) {
        if (!message || Disabled()) return;
        switch (message->type) {
        case SKSEMessagingInterface::kMessage_DataLoaded:
            if (!VerifyEngine()) { InterlockedExchange(&faults, 3); return; }
            InitializeBossRules();
            if (!InstallHUDHooks()) { InterlockedExchange(&faults, 3); Log("HUD message hook unavailable"); return; }
            if (!InstallDamageHooks()) { InterlockedExchange(&faults, 3); Log("health damage hook unavailable"); return; }
            if (!InstallLootNotifications()) Log("loot notification call unavailable; vanilla messages preserved");
            RegisterMenu();
            Log("DataLoaded thread=%lu; load thread=%lu", GetCurrentThreadId(), loadThread);
            break;
        case SKSEMessagingInterface::kMessage_PreLoadGame:
            ResetState(false);
            break;
        case SKSEMessagingInterface::kMessage_PostLoadGame:
            // SKSE passes success in data itself (not a bool pointer).
            ResetState(message->data != nullptr);
            if (message->data) RequestMenuOpen();
            break;
        case SKSEMessagingInterface::kMessage_NewGame:
            ResetState(true);
            RequestMenuOpen();
            break;
        }
    }
    static void OnMessage(SKSEMessagingInterface::Message* message) {
        __try { OnMessageBody(message); }
        __except(Fault("SKSE message", GetExceptionInformation())) { ResetState(false); }
    }
    static bool LoadBody(const SKSEInterface* skse) {
        char path[MAX_PATH];
        loadThread = GetCurrentThreadId();
        if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, path))) {
            strcat_s(path, "\\My Games\\Skyrim\\SKSE\\TrueHUD.log");
            fopen_s(&logFile, path, "w");
        }
        previousPurecall = _set_purecall_handler(OnPurecall);
        Log("TrueHUD LE Test 5 / original Ersh TrueHUD 1.1.10; thread=%lu", loadThread);
        auto messaging = static_cast<SKSEMessagingInterface*>(skse->QueryInterface(kInterface_Messaging));
        auto tasks = static_cast<SKSETaskInterface*>(skse->QueryInterface(kInterface_Task));
        auto papyrus = static_cast<SKSEPapyrusInterface*>(skse->QueryInterface(kInterface_Papyrus));
        if (!messaging || messaging->interfaceVersion < SKSEMessagingInterface::kInterfaceVersion ||
            !tasks || tasks->interfaceVersion < SKSETaskInterface::kInterfaceVersion ||
            !papyrus || !InitializeSettings()) { Log("Required interface/settings initialization failed"); return false; }
        if (!papyrus->Register(RegisterPapyrus)) return false;
        InitializeState(tasks);
        InitializeUI(tasks);
        RegisterEvents(messaging);
        return messaging->RegisterListener(skse->GetPluginHandle(), "SKSE", OnMessage);
    }
}

extern "C" bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info) {
    if (!info || !skse) return false;
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name = "TrueHUD";
    info->version = 1;
    return !skse->isEditor && skse->runtimeVersion == RUNTIME_VERSION_1_9_32_0;
}
extern "C" bool SKSEPlugin_Load(const SKSEInterface* skse) {
    __try { return skse && TrueHUDLE::LoadBody(skse); }
    __except(TrueHUDLE::Fault("SKSEPlugin_Load", GetExceptionInformation())) { return false; }
}
