#pragma once
#include "Runtime.h"
namespace TrueHUDLE {
    struct HUDSignals {
        UInt32 flashes[4] = {};
        bool cartMode = false;
    };
    struct HUDSignal {
        enum Kind { None, Flash, Cart } kind = None;
        UInt32 value = 0;
    };
    bool DecodeHUDSignal(const UIMessage* message, HUDSignal& signal);
    void CopyHUDSignals(HUDSignals& output);
    void ResetHUDSignals();
    bool InstallHUDHooks();
}
