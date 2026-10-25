#include "HUDSignals.h"
#include "Settings.h"
#include <cstring>
namespace TrueHUDLE {
    namespace {
        SRWLOCK signalLock = SRWLOCK_INIT;
        HUDSignals signals;
        using ProcessMessage = UInt32 (__thiscall*)(IMenu*, UIMessage*);
        ProcessMessage originalProcess = nullptr;
        using AdvanceHUD = void (__thiscall*)(IMenu*, UInt32, UInt32);
        AdvanceHUD originalAdvance = nullptr;
        DWORD hudFrameThread = 0;
        using ProcessButton = void (__thiscall*)(void*, void*, void*);
        ProcessButton originalSprint = nullptr;
        DWORD callbackThread = 0;
        volatile LONG sprintThread = 0;
        void __fastcall AdvanceNativeHUD(IMenu* self, void*, UInt32 interval, UInt32 time) {
            __try { originalAdvance(self, interval, time); }
            __except(Fault("native HUD frame original", GetExceptionInformation())) { return; }
            if (Disabled()) return;
            __try {
                if (!hudFrameThread) {
                    hudFrameThread = GetCurrentThreadId();
                    Log("native HUD frame thread=%lu", hudFrameThread);
                }
                OnNativeHUDFrame();
            }
            __except(Fault("native HUD frame bridge", GetExceptionInformation())) {}
        }
        void Observe(const HUDSignal& signal) {
            AcquireSRWLockExclusive(&signalLock);
            if (signal.kind == HUDSignal::Cart) signals.cartMode = signal.value != 0;
            else if (signal.kind == HUDSignal::Flash && signal.value < 4) ++signals.flashes[signal.value];
            ReleaseSRWLockExclusive(&signalLock);
        }
        UInt32 __fastcall ProcessHUDMessage(IMenu* self, void*, UIMessage* message) {
            HUDSignal signal;
            // Copy while the UI message owns its data. Never retain its pointer.
            __try { if (!Disabled()) DecodeHUDSignal(message, signal); }
            __except(Fault("HUD message capture", GetExceptionInformation())) { signal = {}; }
            UInt32 result = 2;
            __try { result = originalProcess(self, message); }
            __except(Fault("HUD message original", GetExceptionInformation())) { return result; }
            if (!Disabled()) {
                if (!callbackThread) { callbackThread = GetCurrentThreadId(); Log("HUD message callback thread=%lu", callbackThread); }
                Observe(signal);
            }
            return result;
        }
        void __fastcall ProcessSprintButton(void* self, void*, void* event, void* controls) {
            // Native 00771B3A..46: ECX handler, event and controls+14 stack
            // words. SprintHandler 00773590 ends RET8, with no return value.
            __try { originalSprint(self, event, controls); }
            __except(Fault("Sprint original", GetExceptionInformation())) { return; }
            if (Disabled()) return;
            __try {
                const DWORD thread = GetCurrentThreadId();
                if (!InterlockedCompareExchange(&sprintThread, static_cast<LONG>(thread), 0))
                    Log("Sprint callback thread=%lu", thread);
                if (GetSetting("bPlayerWidgetDisplayMountStamina:PlayerWidget") && MountExhausted())
                    Observe({HUDSignal::Flash, 2});
            }
            __except(Fault("Sprint mount flash", GetExceptionInformation())) {}
        }
        template <typename Function>
        bool InstallSlot(uintptr_t address, Function& original, void* replacement, const char* label) {
            if (original) return true;
            auto slot = reinterpret_cast<void**>(address);
            if (!Readable(slot, sizeof(void*)) || !Executable(*slot)) return false;
            DWORD protection;
            if (!VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &protection)) return false;
            void* previous;
            do {
                previous = *slot;
                if (!Executable(previous)) {
                    DWORD ignored; VirtualProtect(slot, sizeof(void*), protection, &ignored);
                    return false;
                }
                original = reinterpret_cast<Function>(previous);
            } while (InterlockedCompareExchangePointer(slot, replacement, previous) != previous);
            DWORD ignored;
            const bool restored = VirtualProtect(slot, sizeof(void*), protection, &ignored) != 0;
            Log("%s chain=%p protection-restored=%u", label, previous, restored);
            return true;
        }
    }
    bool DecodeHUDSignal(const UIMessage* message, HUDSignal& signal) {
        signal = {};
        auto names = UIStringHolder::GetSingleton();
        if (!Readable(message, sizeof(UIMessage)) || !Readable(names, sizeof(UIStringHolder)) ||
            !names->hudMenu.data || message->strData.data != names->hudMenu.data ||
            (message->message != 0 && message->message != 7) ||
            !Readable(message->objData, 0x28)) return false;
        const char* data = reinterpret_cast<const char*>(message->objData);
        const UInt32 kind = *reinterpret_cast<const UInt32*>(data + 8);
        if (kind == 0xB) {
            const UInt32 av = *reinterpret_cast<const UInt32*>(data + 0x24);
            if (av >= 24 && av <= 26) signal = {HUDSignal::Flash, av - 24};
            else if (av == 0x26) signal = {HUDSignal::Flash, 3};
        } else if (kind == 0x17) {
            char mode[32];
            CopyString(*reinterpret_cast<const char* const*>(data + 0x0C), mode, sizeof(mode));
            if (!strcmp(mode, "CartMode")) signal = {HUDSignal::Cart, static_cast<UInt32>(data[0x20] != 0)};
        }
        return signal.kind != HUDSignal::None;
    }
    void CopyHUDSignals(HUDSignals& output) {
        AcquireSRWLockShared(&signalLock); output = signals; ReleaseSRWLockShared(&signalLock);
    }
    void ResetHUDSignals() {
        AcquireSRWLockExclusive(&signalLock); signals = {}; ReleaseSRWLockExclusive(&signalLock);
    }
    bool InstallHUDHooks() {
        // HUDMenu ctor installs010E537C; SDK ProcessMessage slot4. UI pump
        // A5D588/A5D7BF calls [vtable+10] with ECX menu, one message word.
        if (!InstallSlot(0x010E538C, originalProcess, reinterpret_cast<void*>(&ProcessHUDMessage), "HUD ProcessMessage")) return false;
        // Same ctor-proven primary table, slot5. Native callerA5CE26..29
        // pushes interval float bits/time; native target85EF60 ends RET8.
        if (!InstallSlot(0x010E5390, originalAdvance, reinterpret_cast<void*>(&AdvanceNativeHUD), "HUD NextFrame")) return false;
        // InputHandler-create.asm0077409F writes010D44FC. Slot4 target773590;
        // retain an already installed executable hook as the next chain member.
        return InstallSlot(0x010D450C, originalSprint, reinterpret_cast<void*>(&ProcessSprintButton), "Sprint ProcessButton");
    }
}
