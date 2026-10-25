#pragma once
#include "Runtime.h"
#include <functional>

namespace TrueHUDLE {
    constexpr UInt32 InvalidResourceOwner = 0xFFFFFFFFu;
    enum class ResourceResult { OK, NotOwner, AlreadyGiven, AlreadyTaken, InvalidArgument };
    using ResourceCallback = std::function<float(Actor*)>;
    struct SpecialResourceStatus {
        UInt32 owner = InvalidResourceOwner, revision = 0;
        bool available = false, specialMode = false, displayPlayer = true;
    };
    ResourceResult RequestSpecialControl(UInt32 owner);
    ResourceResult ReleaseSpecialControl(UInt32 owner);
    ResourceResult RegisterSpecialCallbacks(UInt32 owner, ResourceCallback current, ResourceCallback maximum,
                                           bool specialMode, bool displayPlayer = true);
    SpecialResourceStatus ReadSpecialStatus();
    // Caller holds the Actor handle reference on the game task for the whole call.
    // Provider re-entry may release/replace itself; stale results are never published.
    void SampleSpecialResource(Actor* pinnedActor, bool player, SpecialResourceSample& output);
}
