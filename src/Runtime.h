// TrueHUD LE development port. GPL-3.0-or-later WITH upstream EXCEPTIONS.
#pragma once
#include "skse/PluginAPI.h"
#include "skse/GameReferences.h"
#include "skse/GameMenus.h"
#include "skse/ScaleformLoader.h"
#include "skse/ScaleformMovie.h"
#include <windows.h>
#include <cstdint>

namespace TrueHUDLE
{
    constexpr unsigned MaxActors = 128;
    constexpr unsigned MaxLoot = 32;
    constexpr unsigned MaxFloating = 64;
    struct FloatingSample {
        UInt32 serial = 0, color = 0;
        float amount = 0;
        NiPoint3 position;
    };
    struct LootSample {
        UInt32 serial = 0, formID = 0, count = 0, type = 0;
        char name[256] = {};
        char icon[48] = {};
        UInt32 iconColor = 0xFFFFFF;
    };
    struct SpecialResourceSample {
        float current = 0, maximum = 0;
        UInt32 revision = 0;
        bool available = false, specialMode = false;
    };
    struct ActorSample
    {
        UInt32 handle = 0, formID = 0, soul = 0;
        char name[256] = {};
        float current[3] = {}, maximum[3] = {};
        float screenX = 0, screenY = 0, depth = 0, cameraDistance = 0;
        bool projected = false, visible = true;
        NiPoint3 position, head, body;
        float voiceRecovery = 0;
        float mountStamina = 0, mountMaximumStamina = 0;
        bool mounted = false;
        SpecialResourceSample special;
        UInt16 level = 0;
        bool combat = false, teammate = false, hostile = false, dead = false, boss = false, hit = false;
        bool bleeding = false, essential = false;
        UInt8 manualRole = 0, removal = 255; // 0 auto / 1 info / 2 boss; removal 0..2 or none
    };
    struct Frame
    {
        UInt32 generation = 0, target = 0, count = 0, lootCount = 0;
        UInt32 floatingCount = 0;
        bool ready = false;
        ActorSample player, actors[MaxActors];
        LootSample loot[MaxLoot];
        FloatingSample floating[MaxFloating];
    };

    void Log(const char* format, ...);
    int Fault(const char* context, EXCEPTION_POINTERS* exception);
    bool Disabled();
    bool Readable(const void* pointer, size_t length, bool object = true);
    bool Executable(const void* pointer);
    void CopyString(const char* source, char* output, size_t capacity);
    bool SampleActor(UInt32 handle, ActorSample& sample, bool target = false);
    bool IsPlayerAlly(Actor* actor);
    bool Project(const NiPoint3& world, float& x, float& y, float& depth, float* cameraDistance = nullptr);
    bool VerifyEngine();
    void InitializeState(SKSETaskInterface* tasks);
    void RegisterEvents(SKSEMessagingInterface* messaging);
    void RequestFrame();
    void CopyFrame(Frame& output);
    bool MountExhausted();
    void ResetState(bool enabled);
    void RegisterMenu();
    void InitializeUI(SKSETaskInterface* tasks);
    void RequestMenuOpen();
    void OnNativeHUDFrame();
    bool RegisterPapyrus(VMClassRegistry* registry);
    void InitializeBossRules();
    bool IsBoss(Actor* actor);
    UInt32 OriginalBaseID(const Actor* pinnedActor);
    void PrepareLocationBosses(Actor* player);
    void ForgetActor(UInt32 handle);
    void ApplyActorCommand(UInt32 handle, bool boss, bool remove, UInt32 removalMode);
    void SetNativeTarget(UInt32 handle, bool available);
    bool ReadNativeTarget(const HUDMenu* hud, UInt32& handle);
    struct ChargeSample { float percent = 0; bool show = false; };
    void ReadNativeCharges(ChargeSample output[2]);
}
