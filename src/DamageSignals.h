#pragma once
#include "Runtime.h"
namespace TrueHUDLE {
    struct DamageEvent {
        UInt32 serial = 0, handle = 0, color = 0, generation = 0;
        float amount = 0;
    };
    bool InstallDamageHooks();
    void ResetDamageSignals(bool enabled);
    unsigned DrainDamageSignals(DamageEvent* output, unsigned capacity);
}
