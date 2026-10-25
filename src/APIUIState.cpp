#include "APIUIState.h"
#include "Runtime.h"
#include <cstring>

namespace TrueHUDLE {
    namespace {
        volatile LONG apiThread = 0;
        SRWLOCK barsLock = SRWLOCK_INIT;
        UInt32 infoHandles[MaxActors];
        bool infoBosses[MaxActors];
        unsigned infoCount = 0;
        struct Profile { UInt32 actor = 0, mask = 0, revision = 0, colors[20] = {}; } profiles[MaxActors + 1];
        UInt32 colorRevision = 0; // UI thread only; no owning engine pointers.
        double Settings::* const colorMembers[] = {
            &Settings::uHealthColor, &Settings::uHealthPhantomColor, &Settings::uHealthBackgroundColor,
            &Settings::uHealthPenaltyColor, &Settings::uHealthFlashColor,
            &Settings::uMagickaColor, &Settings::uMagickaPhantomColor, &Settings::uMagickaBackgroundColor,
            &Settings::uMagickaPenaltyColor, &Settings::uMagickaFlashColor,
            &Settings::uStaminaColor, &Settings::uStaminaPhantomColor, &Settings::uStaminaBackgroundColor,
            &Settings::uStaminaPenaltyColor, &Settings::uStaminaFlashColor,
            &Settings::uSpecialColor, &Settings::uSpecialPhantomColor, &Settings::uSpecialBackgroundColor,
            &Settings::uSpecialPenaltyColor, &Settings::uSpecialFlashColor
        };
    }
    void SetAPIThread(DWORD thread) { InterlockedExchange(&apiThread, static_cast<LONG>(thread)); }
    DWORD ReadAPIThread() { return static_cast<DWORD>(InterlockedCompareExchange(&apiThread, 0, 0)); }
    void PublishInfoBars(const UInt32* handles, const bool* bosses, unsigned count) {
        if (count > MaxActors) count = MaxActors;
        AcquireSRWLockExclusive(&barsLock);
        infoCount = count;
        if (count) { memcpy(infoHandles, handles, count * sizeof(UInt32)); memcpy(infoBosses, bosses, count * sizeof(bool)); }
        ReleaseSRWLockExclusive(&barsLock);
    }
    bool HasPublishedInfoBar(UInt32 handle, bool floatingOnly) {
        if (!handle) return false;
        bool found = false;
        AcquireSRWLockShared(&barsLock);
        for (unsigned i = 0; i < infoCount; ++i)
            if (infoHandles[i] == handle && (!floatingOnly || !infoBosses[i])) { found = true; break; }
        ReleaseSRWLockShared(&barsLock);
        return found;
    }
    void ResetAPIColors() { memset(profiles, 0, sizeof(profiles)); ++colorRevision; }
    void ProcessAPIColor(const APICommand& command) {
        if (!command.actor || command.type >= 4 || command.id >= 5) return;
        Profile* selected = nullptr;
        for (auto& profile : profiles) {
            if (profile.actor == command.actor) { selected = &profile; break; }
            if (!profile.actor && !selected) selected = &profile;
        }
        if (!selected) { Log("TrueHUD color profile limit reached"); return; }
        if (!selected->actor && command.kind == APICommandKind::RevertColor) return;
        selected->actor = command.actor;
        const unsigned index = command.type * 5 + command.id;
        if (command.kind == APICommandKind::OverrideColor) {
            selected->mask |= 1u << index; selected->colors[index] = command.value & 0xFFFFFF;
        } else selected->mask &= ~(1u << index);
        selected->revision = ++colorRevision;
        // No remaining overrides means the defaults are authoritative again.
        if (!selected->mask) *selected = {};
    }
    UInt32 CopyAPIColors(UInt32 handle, Settings& settings) {
        for (const auto& profile : profiles) if (profile.actor == handle && handle) {
            for (unsigned i = 0; i < 20; ++i) if (profile.mask & (1u << i)) settings.*colorMembers[i] = profile.colors[i];
            return profile.revision;
        }
        return 0;
    }
}
