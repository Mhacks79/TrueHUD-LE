#pragma once
#include "Settings.h"
#include "APICommands.h"
namespace TrueHUDLE {
    void* RequestAPI(TRUEHUD_API::InterfaceVersion version) noexcept;
    void SetAPIThread(DWORD thread);
    DWORD ReadAPIThread();
    void PublishInfoBars(const UInt32* handles, const bool* bosses, unsigned count);
    bool HasPublishedInfoBar(UInt32 handle, bool floatingOnly);
    void ResetAPIColors();
    void ProcessAPIColor(const APICommand& command);
    UInt32 CopyAPIColors(UInt32 handle, Settings& settings);
    UInt32 ReadAPIColorRevision(); // UI thread; changes only when a profile changes
    struct ColorRevisionMemo { UInt32 actor = 0, epoch = 0, revision = 0; };
    UInt32 ReadAPIColorRevision(UInt32 actor, ColorRevisionMemo& memo);
}
