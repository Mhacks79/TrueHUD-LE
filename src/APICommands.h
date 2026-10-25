#pragma once
#include "Runtime.h"
#include "../include/TrueHUDAPI.h"

namespace TrueHUDLE {
    enum class APICommandKind {
        AddActor, RemoveActor, AddBoss, RemoveBoss, FlashActor, FlashSpecial,
        LoadCustom, RegisterCustomType, AddCustom, RemoveCustom, OverrideColor, RevertColor
    };
    struct APICommand {
        APICommandKind kind = APICommandKind::AddActor;
        UInt32 owner = 0, actor = 0, type = 0, id = 0, value = 0, generation = 0;
        bool longFlash = false;
        char text[260] = {};
        std::shared_ptr<TRUEHUD_API::WidgetBase> widget;
        TRUEHUD_API::APIResultCallback callback;
    };
    constexpr unsigned MaxAPICommands = 256;
    bool EnqueueAPICommand(APICommand command);
    // UI consumer only: destruction of previous result occurs outside queue lock.
    bool PopAPICommand(APICommand& output, bool& currentGeneration);
    void ResetAPICommands();
}
