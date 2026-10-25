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
}
