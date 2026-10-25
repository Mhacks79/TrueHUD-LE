#include "Runtime.h"
#include "LootIcons.h"
#include "LootNotifications.h"
#include "Settings.h"
#include <cstring>

namespace TrueHUDLE {
    namespace {
        volatile LONG replacementReady = 0, callbackThread = 0;
        using FormatMessage = void (__cdecl*)(const char*,SInt32,UInt32);
        FormatMessage originalFormat = nullptr;
        bool ShouldSuppress(TESForm* object,SInt32 count,UInt32 added) {
            if (Disabled() || count<=0 || !(added&0xFF) || !InterlockedCompareExchange(&replacementReady,0,0) ||
                !GetSetting("bEnableRecentLoot:General") || !GetSetting("bRecentLootHideVanillaMessage:RecentLoot")) return false;
            LootSample sample;
            return ClassifyLoot(object,sample);
        }
        void __cdecl FormatNotificationBody(TESForm* object,const char* name,SInt32 count,UInt32 added) {
            const LONG thread=static_cast<LONG>(GetCurrentThreadId());
            if (!InterlockedCompareExchange(&callbackThread,thread,0)) Log("loot notification callback thread=%lu",thread);
            bool suppress=false;
            __try { suppress=ShouldSuppress(object,count,added); }
            __except(Fault("loot notification decision",GetExceptionInformation())) {}
            if (suppress) return;
            __try { originalFormat(name,count,added); }
            __except(Fault("loot notification original",GetExceptionInformation())) {}
        }
        // At the sole native CALL00861100, EBX is the object saved at008610D1.
        // The caller owns three cdecl words, ADD ESP,0C at00861105. Capture EBX
        // in this exact-site bridge; do not infer it in an optimized C++ frame.
        __declspec(naked) void FormatNotificationBridge() {
            __asm {
                push ebp
                mov ebp,esp
                push dword ptr [ebp+16]
                push dword ptr [ebp+12]
                push dword ptr [ebp+8]
                push ebx
                call FormatNotificationBody
                add esp,16
                pop ebp
                ret
            }
        }
    }
    void SetLootReplacementReady(bool ready) { InterlockedExchange(&replacementReady,ready?1:0); }
    bool InstallLootNotifications() {
        if (originalFormat) return true;
        auto site=reinterpret_cast<unsigned char*>(0x00861100);
        if (!Readable(site,5,false) || site[0]!=0xE8) return false;
        SInt32 displacement; memcpy(&displacement,site+1,sizeof(displacement));
        auto previous=reinterpret_cast<void*>(uintptr_t(site)+5+static_cast<uintptr_t>(displacement));
        if (!Executable(previous)) return false;
        DWORD protection;
        if (!VirtualProtect(site,5,PAGE_EXECUTE_READWRITE,&protection)) return false;
        originalFormat=reinterpret_cast<FormatMessage>(previous);
        const UInt32 replacement=static_cast<UInt32>(reinterpret_cast<uintptr_t>(&FormatNotificationBridge)-(uintptr_t(site)+5));
        memcpy(site+1,&replacement,sizeof(replacement));
        FlushInstructionCache(GetCurrentProcess(),site,5);
        DWORD ignored; const bool restored=VirtualProtect(site,5,protection,&ignored)!=0;
        Log("loot formatter call chain=%p protection-restored=%u; native pickup sound untouched",previous,restored);
        return true;
    }
}
