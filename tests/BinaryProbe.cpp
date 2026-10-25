// Query the actual built DLL in an isolated x86 process. Never invoke Load.
#include <windows.h>
#include "skse/PluginAPI.h"
#include "skse/skse_version.h"
#include "../include/TrueHUDAPI.h"
#include <cstdio>
#include <cstring>

int wmain(int argc, wchar_t** argv) {
    if (argc != 2) return 2;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    HMODULE module = LoadLibraryW(argv[1]);
    if (!module) { printf("LoadLibrary failed: %lu\n", GetLastError()); return 1; }
    using Query = bool (__cdecl*)(const SKSEInterface*, PluginInfo*);
    auto query = reinterpret_cast<Query>(GetProcAddress(module, "SKSEPlugin_Query"));
    if (!query || !GetProcAddress(module, "SKSEPlugin_Load")) return 1;
    SKSEInterface skse{};
    skse.runtimeVersion = RUNTIME_VERSION_1_9_32_0;
    struct GuardedInfo { UInt32 before = 0xAABBCCDD; PluginInfo info{}; UInt32 after = 0x55667788; } result;
    unsigned checks = 0, failures = 0;
    auto check = [&](bool ok, const char* label) { ++checks; if (!ok) { ++failures; printf("FAIL %s\n", label); } };
    check(query(&skse, &result.info), "LE19320 accepted");
    check(result.info.infoVersion == PluginInfo::kInfoVersion && result.info.name &&
          !strcmp(result.info.name, "TrueHUD"), "metadata readable from real DLL");
    check(result.before == 0xAABBCCDD && result.after == 0x55667788, "x86 PluginInfo ABI canaries");
    skse.isEditor = 1;
    check(!query(&skse, &result.info), "editor refused");
    skse.isEditor = 0; skse.runtimeVersion = 0x01090300;
    check(!query(&skse, &result.info), "unsupported runtime refused");
    check(!query(nullptr, &result.info) && !query(&skse, nullptr), "null query arguments refused");
    using namespace TRUEHUD_API;
    auto request = reinterpret_cast<_RequestPluginAPI>(GetProcAddress(module, "RequestPluginAPI"));
    check(request != nullptr, "public cdecl export present in actual DLL");
    if (request) {
        auto api = static_cast<IVTrueHUD2*>(request(InterfaceVersion::V2));
        check(api && request(InterfaceVersion::V1) == api, "real Win32 consumer receives v1 and v2 vtables");
        check(request(InterfaceVersion::V3) && request(InterfaceVersion::V4) && !request(static_cast<InterfaceVersion>(255)), "four implemented versions and unknown version refusal");
        if (api) {
            check(api->GetTrueHUDThreadId() == 0, "UI thread is unknown before actual menu callback");
            check(api->RequestTargetControl(17) == APIResult::OK && api->GetTargetControlOwner() == 17 &&
                api->RequestTargetControl(18) == APIResult::AlreadyTaken, "cross-DLL target vtable ownership calls");
            check(api->ReleaseTargetControl(18) == APIResult::NotOwner && api->ReleaseTargetControl(17) == APIResult::OK,
                "cross-DLL target release preserves owner");
            check(api->RequestSpecialResourceBarsControl(17) == APIResult::OK &&
                api->GetSpecialResourceBarControlOwner() == 17, "cross-DLL special resource ownership");
            SpecialResourceCallback current = [](Actor*) { return 12.f; }, maximum = [](Actor*) { return 20.f; };
            check(api->RegisterSpecialResourceFunctions(17, std::move(current), std::move(maximum), true, false) == APIResult::OK,
                "cross-DLL release STL callback registration ABI");
            check(api->ReleaseSpecialResourceBarControl(17) == APIResult::OK &&
                api->GetSpecialResourceBarControlOwner() == kPluginHandle_Invalid, "cross-DLL callback release before module unload");
        }
    }
    FreeLibrary(module);
    printf("actual DLL Query/API consumer probe: %u checks, %u failures; Load/game callbacks not invoked\n", checks, failures);
    return failures ? 1 : 0;
}
