#include "Runtime.h"
#include "Settings.h"
#include "HUDOverride.h"
#include "WidgetPolicy.h"
#include "HUDSignals.h"
#include "SpecialResources.h"
#include "TargetControl.h"
#include "APICommands.h"
#include "APIUIState.h"
#include "CustomWidgets.h"
#include "DebugDraw.h"
#include "DamageSignals.h"
#include "LootIcons.h"
#include "LootNotifications.h"
#include "ProcessLifetime.h"
#include "skse/GameObjects.h"
#include "../include/TrueHUDAPI.h"
#include <memory>
#include "skse/GameAPI.h"
#include "skse/GameRTTI.h"
#include "skse/GameThreads.h"
#include "skse/PapyrusEvents.h"
#include "skse/NiNodes.h"
#include "skse/GameData.h"
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <cstring>
#include <chrono>
#include <clocale>
#include <limits>
#include "../harness/fakegame.h"

static const unsigned long gameBlocks[] = {
    0x00400000, 0x00430000, 0x00450000, 0x004A0000, 0x004D0000,
    0x00590000, 0x00650000, 0x006A0000, 0x006D0000, 0x006E0000, 0x00AB0000, 0x00F50000,
    0x00850000, 0x00860000, 0x008C0000, 0x00A50000, 0x00A60000, 0x010D0000, 0x010E0000, 0x012B0000, 0x012E0000, 0x01310000, 0x01B20000, 0x01B30000
};
FAKEGAME_CLAIM(gameBlocks)

namespace {
    unsigned lateSinkDestructions = 0;
    struct LateSinkBase { virtual ~LateSinkBase() = default; virtual int Receive() = 0; };
    struct LateSink final : LateSinkBase {
        ~LateSink() override { ++lateSinkDestructions; }
        int Receive() override { return 73; }
    };
    LateSinkBase* lateSinkAddress = nullptr;
    void CheckLateSinkAtExit() {
        if (!lateSinkAddress) return; // supervisor parent does not run the tests
        if (lateSinkDestructions || lateSinkAddress->Receive() != 73) {
            printf("  FAIL  registered sink destroyed before late engine callback\n1 checks, 1 failures\n");
            ExitProcess(1); // deterministic rejection, never invoke an abstract slot
        }
        printf("late-exit registered callback: 1 checks, 0 failures\n");
    }
    struct EarlyExitObserver { EarlyExitObserver() { atexit(CheckLateSinkAtExit); } } earlyExitObserver;
    TrueHUDLE::ProcessLifetime<LateSink> lateSinkStorage;
    static_assert(std::is_trivially_destructible<decltype(lateSinkStorage)>::value,
        "registered sink storage must not schedule a CRT destructor");
    struct NumberFixture {
        struct Movie { double number; unsigned writes = 0; bool exists = true; };
        static bool Read(Movie* movie, const char*, double& number) { number = movie->number; return movie->exists; }
        static void Write(Movie* movie, const char*, double number) { movie->number = number; ++movie->writes; }
    };
    int failures = 0, checks = 0, faults = 0;
    int lookupCalls = 0, modifierCalls = 0, wrongSelf = 0, wrongMaximumCalls = 0;
    int capturedSinks = 0;
    bool forceFault = false;
    bool resetDuringSample = false;
    bool sentientSoul = false;
    bool invisibleActor = false, hasLOS = true, losFault = false;
    bool bulkHandles = false;
    bool mountedActor = false, mountSelf = false, mountFault = false;
    bool emptyMount = false;
    bool projectionNaN = false, projectionReject = false;
    unsigned losCalls = 0;
    alignas(16) unsigned char secondPlayerBytes[sizeof(PlayerCharacter)] = {}, losGraphBytes[0xBC] = {}, losViewBytes[0x50] = {};
    const void* losChildren[1] = {losViewBytes};
    alignas(16) unsigned char keywordBytes[sizeof(BGSKeywordForm)] = {}, keywordFormBytes[sizeof(TESForm)] = {};
    BGSKeyword* keywordPointers[1];
    TrueHUDLE::Frame fixtureFrame;
    TrueHUDLE::DebugLine debugFixture[TrueHUDLE::MaxDebugLines];
    alignas(16) unsigned char hudBytes[sizeof(HUDMenu)] = {}, enemyBytes[sizeof(EnemyHealth)] = {};
    HUDObject* hudParts[2];
    alignas(16) unsigned char stringHolderBytes[sizeof(UIStringHolder)] = {}, messageBytes[sizeof(UIMessage)] = {}, hudDataBytes[0x28] = {};
    int originalMessageCalls = 0;
    int originalAdvanceCalls = 0;
    void __fastcall OriginalHUDFrame(IMenu* self, void*, UInt32 interval, UInt32 time) {
        if (self != reinterpret_cast<IMenu*>(hudBytes) || interval != 0x3C888889 || time != 0x55667788) ++wrongSelf;
        ++originalAdvanceCalls;
    }
    int originalSprintCalls = 0;
    void __fastcall OriginalSprint(void* self, void*, void* event, void* controls) {
        if (self != reinterpret_cast<void*>(0x11110000) || event != reinterpret_cast<void*>(0x22220000) ||
            controls != reinterpret_cast<void*>(0x33330000)) ++wrongSelf;
        ++originalSprintCalls;
    }
    void Sprint() {
        using Call = void (__thiscall*)(void*, void*, void*);
        reinterpret_cast<Call>(*reinterpret_cast<void**>(0x010D450C))(
            reinterpret_cast<void*>(0x11110000), reinterpret_cast<void*>(0x22220000), reinterpret_cast<void*>(0x33330000));
    }
    UInt32 __fastcall OriginalHUDMessage(IMenu* self, void*, UIMessage* message) {
        if (self != reinterpret_cast<IMenu*>(hudBytes) || message != reinterpret_cast<UIMessage*>(messageBytes)) ++wrongSelf;
        ++originalMessageCalls; return 0x1234;
    }
    alignas(16) unsigned char dataBytes[sizeof(DataHandler)] = {}, modBytes[sizeof(ModInfo)] = {};
    alignas(16) unsigned char baseBytes[sizeof(TESForm)] = {}, raceBytes[sizeof(TESForm)] = {};
    alignas(16) unsigned char lootFormBytes[sizeof(TESObjectWEAP)] = {}, lootNameBytes[sizeof(TESFullName)] = {};
    alignas(16) unsigned char locationBytes[sizeof(BGSLocation)] = {}, refTypeBytes[sizeof(TESForm)] = {};
    struct LocationRefFixture { TESForm* type; UInt32 id, padding[2]; } locationRefs[1];
    Actor* actor;
    bool ExpectedActor(const Actor* value) { return value == actor || value == reinterpret_cast<Actor*>(secondPlayerBytes); }
    alignas(16) unsigned char actorBytes[sizeof(PlayerCharacter)] = {};
    alignas(16) unsigned char processBytes[0xA0] = {}, middleBytes[0x210] = {}, highBytes[0x40] = {};
    alignas(16) unsigned char loadedBytes[0x40] = {}, nodeBytes[0x100] = {};
    void* actorTable[0x140] = {}, *avTable[10] = {};
    TaskDelegate* scheduled = nullptr;
    BSTEventSink<TESCombatEvent>* combat = nullptr;
    BSTEventSink<TESHitEvent>* hit = nullptr;
    BSTEventSink<SKSECrosshairRefEvent>* crosshair = nullptr;
    BSTEventSink<TESContainerChangedEvent>* container = nullptr;

    void Check(bool condition, const char* label) {
        ++checks;
        if (!condition) ++failures;
        FakeGame::Check(condition, "%s", label);
    }
    void TestSpecialResources() {
        using namespace TrueHUDLE;
        Check(RequestSpecialControl(InvalidResourceOwner) == ResourceResult::InvalidArgument,
              "invalid owner cannot claim special resources");
        Check(RequestSpecialControl(7) == ResourceResult::OK && RequestSpecialControl(7) == ResourceResult::AlreadyGiven &&
              RequestSpecialControl(8) == ResourceResult::AlreadyTaken, "special ownership is exclusive and idempotent");
        Check(RegisterSpecialCallbacks(8, [](Actor*) { return 1.f; }, [](Actor*) { return 2.f; }, false) == ResourceResult::NotOwner &&
              RegisterSpecialCallbacks(7, {}, {}, false) == ResourceResult::InvalidArgument, "special callbacks require ownership and both functions");
        bool pinned = false;
        RegisterSpecialCallbacks(7, [&](Actor* value) { pinned = value == actor && value->handleRefObject.GetRefCount() > 1; return 45.f; },
                                   [](Actor*) { return 100.f; }, true);
        ActorSample sampled;
        Check(SampleActor(7, sampled) && pinned && sampled.special.available && sampled.special.specialMode &&
              sampled.special.current == 45 && sampled.special.maximum == 100, "provider runs inside real actor pin and publishes only plain values");
        RegisterSpecialCallbacks(7, [](Actor*) { return 9.f; }, [](Actor*) { return 10.f; }, false, false);
        SpecialResourceSample special;
        SampleSpecialResource(actor, true, special);
        Check(!special.available && special.current == 0, "player resource opt-out is respected");
        SampleSpecialResource(actor, false, special);
        Check(special.available && special.current == 9, "player opt-out does not disable actor resources");
        RegisterSpecialCallbacks(7, [](Actor*) { return -9.f; }, [](Actor*) { return -10.f; }, false);
        SampleSpecialResource(actor, false, special);
        Check(special.available && special.current == 0 && special.maximum == 0, "negative provider values clamp to zero");
        RegisterSpecialCallbacks(7, [](Actor*) { return 150.f; }, [](Actor*) { return 100.f; }, false);
        SampleSpecialResource(actor, false, special);
        Check(special.current == 150 && special.maximum == 100, "over-cap current value retained for upstream percent clamp");
        RegisterSpecialCallbacks(7, [](Actor*) {
            RegisterSpecialCallbacks(7, [](Actor*) { return 22.f; }, [](Actor*) { return 50.f; }, true);
            return 3.f;
        }, [](Actor*) { return 4.f; }, false);
        SampleSpecialResource(actor, false, special);
        Check(!special.available && special.current == 0, "reentrant provider replacement discards old callback result");
        SampleSpecialResource(actor, false, special);
        Check(special.available && special.current == 22 && special.maximum == 50 && special.specialMode,
              "replacement snapshot uses a consistent callback pair");
        RegisterSpecialCallbacks(7, [](Actor*) { ReleaseSpecialControl(7); return 1.f; }, [](Actor*) { return 2.f; }, false);
        SampleSpecialResource(actor, false, special);
        Check(!special.available && ReadSpecialStatus().owner == InvalidResourceOwner,
              "provider can release itself without deadlock or stale publication");
        RequestSpecialControl(7);
        unsigned calls = 0;
        RegisterSpecialCallbacks(7, [&](Actor*) -> float { ++calls; throw 42; }, [](Actor*) { return 2.f; }, false);
        for (unsigned i = 0; i < 4; ++i) SampleSpecialResource(actor, false, special);
        Check(calls == 3 && !special.available && !ReadSpecialStatus().available && faults == 0,
              "three C++ callback faults disable provider without disabling native HUD");
        calls = 0;
        RegisterSpecialCallbacks(7, [&](Actor*) { ++calls; return std::numeric_limits<float>::quiet_NaN(); },
                                   [](Actor*) { return 2.f; }, false);
        for (unsigned i = 0; i < 4; ++i) SampleSpecialResource(actor, false, special);
        Check(calls == 3 && !special.available, "non-finite results stop after bounded failure budget");
        calls = 0;
        RegisterSpecialCallbacks(7, [&](Actor*) -> float { ++calls; RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, nullptr); return 0; },
                                   [](Actor*) { return 2.f; }, false);
        for (unsigned i = 0; i < 4; ++i) SampleSpecialResource(actor, false, special);
        Check(calls == 3 && !special.available && faults == 0, "SEH provider faults are contained with independent bounded budget");
        unsigned destroyed = 0;
        struct ReentrantDestructor {
            unsigned* count;
            explicit ReentrantDestructor(unsigned* value) : count(value) {}
            ~ReentrantDestructor() { ReadSpecialStatus(); ++*count; }
        };
        auto lifetime = std::make_shared<ReentrantDestructor>(&destroyed);
        RegisterSpecialCallbacks(7, [lifetime](Actor*) { return 1.f; }, [](Actor*) { return 2.f; }, false);
        lifetime.reset();
        Check(ReleaseSpecialControl(8) == ResourceResult::NotOwner && ReleaseSpecialControl(7) == ResourceResult::OK && destroyed == 1,
              "release destroys captured client objects outside registry lock");
        Check(!ReadSpecialStatus().available && ReadSpecialStatus().owner == InvalidResourceOwner,
              "released provider leaves no resource callbacks active");
    }
    void TestWidgetDelegates() {
        class ClientWidget final : public TRUEHUD_API::WidgetBase {
        public:
            void Update(float) override {}
            void Initialize() override {}
            void Dispose() override {}
        } widget;
        Check(widget._widgetID == 0 && widget._widgetState == TRUEHUD_API::WidgetBase::kHidden,
              "LE public WidgetBase initializes ID and state");
        unsigned calls = 0;
        std::function<void()> repeat;
        repeat = [&] { ++calls; if (calls < 70) widget.AddWidgetTask(repeat); };
        widget.AddWidgetTask(repeat);
        widget.ProcessDelegates();
        Check(calls == 64 && widget._taskQueue.size() == 1,
              "client self-requeued delegates cannot monopolize one UI frame");
        widget.ProcessDelegates();
        Check(calls == 70 && widget._taskQueue.empty(), "deferred client work resumes next frame without loss");
        for (unsigned i = 0; i < 300; ++i) widget.AddWidgetTask([&] { ++calls; });
        Check(widget._taskQueue.size() == 256, "client delegate queue has a fixed admission bound");
        while (!widget._taskQueue.empty()) widget.ProcessDelegates();
        Check(calls == 326, "admitted client delegates execute exactly once");
    }
    void TestAPIQueue() {
        using namespace TrueHUDLE;
        APICommand command, output;
        bool current;
        unsigned admitted = 0;
        for (unsigned i = 0; i < MaxAPICommands; ++i) {
            command.id = i;
            admitted += EnqueueAPICommand(command) ? 1 : 0;
        }
        Check(admitted == MaxAPICommands && !EnqueueAPICommand(command), "API queue admits its capacity and rejects overflow");
        bool ordered = true;
        for (unsigned i = 0; i < MaxAPICommands; ++i) {
            ordered &= PopAPICommand(output, current) && current && output.id == i;
        }
        Check(ordered && !PopAPICommand(output, current), "API queue preserves FIFO order across a full drain");
        command.kind = APICommandKind::AddCustom;
        strcpy_s(command.text, "OriginalSymbol");
        EnqueueAPICommand(command);
        strcpy_s(command.text, "ChangedSymbol");
        PopAPICommand(output, current);
        Check(!strcmp(output.text, "OriginalSymbol"), "queued symbol/path strings own their copied bytes");
        command.kind = APICommandKind::AddCustom; EnqueueAPICommand(command);
        command.kind = APICommandKind::LoadCustom; EnqueueAPICommand(command);
        command.kind = APICommandKind::RegisterCustomType; EnqueueAPICommand(command);
        ResetAPICommands();
        PopAPICommand(output, current);
        Check(!current, "load reset invalidates transient API widget commands");
        PopAPICommand(output, current);
        Check(current && output.kind == APICommandKind::LoadCustom, "library definition requests survive game load");
        PopAPICommand(output, current);
        Check(current && output.kind == APICommandKind::RegisterCustomType, "widget type definition requests survive game load");
        unsigned destroyed = 0, failedReentry = 0;
        struct ReentrantCapture {
            unsigned* destroyed;
            unsigned* failed;
            ReentrantCapture(unsigned* count, unsigned* failures) : destroyed(count), failed(failures) {}
            ~ReentrantCapture() {
                ++*destroyed;
                try {
                    APICommand nested; nested.id = 77;
                    EnqueueAPICommand(std::move(nested));
                } catch (...) { ++*failed; } // report the restored lock bug instead of noexcept terminate
            }
        };
        auto token = std::make_shared<ReentrantCapture>(&destroyed, &failedReentry);
        command = {};
        command.callback = [token](TRUEHUD_API::APIResult) {};
        EnqueueAPICommand(std::move(command)); token.reset();
        PopAPICommand(output, current);
        PopAPICommand(output, current); // destroys last capture before acquiring queue lock
        Check(destroyed == 1 && failedReentry == 0 && output.id == 77 && current,
              "popped callback capture can enqueue during destruction without deadlock");
        Check(!PopAPICommand(output, current), "reentrant API queue drain leaves no duplicate jobs");
    }
    void Install(UInt32 address, void* target) {
        unsigned char bytes[5] = {0xE9};
        const UInt32 relative = reinterpret_cast<UInt32>(target) - address - 5;
        memcpy(bytes + 1, &relative, 4);
        memcpy(reinterpret_cast<void*>(address), bytes, sizeof(bytes));
    }
    UInt16 __fastcall Level(Actor* self, void*) {
        if (!ExpectedActor(self)) ++wrongSelf;
        if (resetDuringSample) { resetDuringSample = false; TrueHUDLE::ResetState(false); }
        if (forceFault) RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, nullptr);
        return 20;
    }
    bool __fastcall Combat(Actor* self, void*) { if (!ExpectedActor(self)) ++wrongSelf; return true; }
    bool __fastcall Hostile(Actor* self, void*, Actor*) { if (!ExpectedActor(self)) ++wrongSelf; return false; }
    const char* __fastcall Name(Actor* self, void*) { if (!ExpectedActor(self)) ++wrongSelf; return "LE fixture"; }
    float __fastcall Current(void* self, void*, UInt32 av) {
        FakeGame::NoteStackDepth();
        if (self != reinterpret_cast<char*>(actor) + 0x60 && self != reinterpret_cast<char*>(secondPlayerBytes) + 0x60) ++wrongSelf;
        if (emptyMount && self == reinterpret_cast<char*>(secondPlayerBytes) + 0x60 && av == 26) return 0;
        return av == 0x36 ? (invisibleActor ? 100.f : 0.f) : av == 24 ? 135.f : av == 25 ? 40.f : 75.f;
    }
    float __fastcall WrongMaximum(void*, void*, UInt32) { ++wrongMaximumCalls; return 100.f; }
    float __fastcall Modifier(Actor* self, void*, UInt32 which, UInt32 av) {
        if (!ExpectedActor(self) || which != 2 || av < 24 || av > 26) ++wrongSelf;
        ++modifierCalls;
        if (emptyMount && self == reinterpret_cast<Actor*>(secondPlayerBytes) && av == 26) return -100;
        return av == 24 ? -15.f : av == 25 ? -10.f : -25.f;
    }
    bool __cdecl Lookup(UInt32* handle, TESObjectREFR** output) {
        ++lookupCalls;
        if (*handle == 8) {
            *output = reinterpret_cast<TESObjectREFR*>(secondPlayerBytes);
            InterlockedIncrement(&(*output)->handleRefObject.m_uiRefCount); return true;
        }
        if (*handle != 7 && !(bulkHandles && *handle >= 100)) { *output = nullptr; return false; }
        *output = actor;
        InterlockedIncrement(&actor->handleRefObject.m_uiRefCount);
        return true;
    }
    void __cdecl CreateHandle(UInt32* handle, TESObjectREFR* ref) {
        if (ref == reinterpret_cast<TESObjectREFR*>(secondPlayerBytes)) { *handle = 8; return; }
        if (ref != actor) ++wrongSelf;
        *handle = 7;
    }
    bool __cdecl Projection(const float* matrix, const NiRect<float>* viewport, const NiPoint3*,
                            float* x, float* y, float* z, float) {
        if (matrix != reinterpret_cast<float*>(0x01B3EA10) ||
            viewport != reinterpret_cast<NiRect<float>*>(0x01B3EA74)) ++wrongSelf;
        *x = projectionNaN ? std::numeric_limits<float>::quiet_NaN() : .4f;
        *y = .6f; *z = .8f; return !projectionReject;
    }
    void __fastcall CaptureSink(void* dispatcher, void*, void* sink) {
        if (dispatcher == reinterpret_cast<void*>(0x012E4DB0)) combat = static_cast<BSTEventSink<TESCombatEvent>*>(sink);
        else if (dispatcher == reinterpret_cast<void*>(0x012E4F60)) hit = static_cast<BSTEventSink<TESHitEvent>*>(sink);
        else if (dispatcher == reinterpret_cast<void*>(0x012E4DE0)) container = static_cast<BSTEventSink<TESContainerChangedEvent>*>(sink);
        else crosshair = static_cast<BSTEventSink<SKSECrosshairRefEvent>*>(sink);
        ++capturedSinks;
    }
    TESForm* __cdecl LookupForm(UInt32 id) { return id == 0x1234 ? reinterpret_cast<TESForm*>(lootFormBytes) : nullptr; }
    void* __cdecl CastForm(void* form, UInt32, const void*, const void* targetType, UInt32) {
        if (form == lootFormBytes && targetType == RTTI_TESFullName) return lootNameBytes;
        return form == actor->baseForm && targetType == RTTI_BGSKeywordForm ? keywordBytes : nullptr;
    }
    UInt8 __fastcall Sentient(Actor* self, void*) { if (!ExpectedActor(self)) ++wrongSelf; return sentientSoul; }
    UInt32 __cdecl SoulType(UInt32 level, UInt32 black) { if (level != 20 || black) ++wrongSelf; return 3; }
    bool __cdecl FixtureLOS(Actor* player, TESObjectREFR* target, UInt8* detail) {
        if (player != *g_thePlayer || !ExpectedActor(static_cast<Actor*>(target)) || !detail) ++wrongSelf;
        if (losFault) RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, nullptr);
        ++losCalls; *detail = true; return hasLOS;
    }
    bool __fastcall GetMount(Actor* self, void*, Actor** output) {
        if (!ExpectedActor(self)) ++wrongSelf;
        *output = mountedActor ? (mountSelf ? self : reinterpret_cast<Actor*>(secondPlayerBytes)) : nullptr;
        if (*output) InterlockedIncrement(&(*output)->handleRefObject.m_uiRefCount);
        if (mountFault) RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, nullptr);
        return *output != nullptr;
    }
    void __cdecl Schedule(TaskDelegate* task) { Check(!scheduled, "one game delegate in flight"); scheduled = task; }
    void* __cdecl Dispatcher(UInt32) { return reinterpret_cast<void*>(0x02010000); }
    void Drain() {
        auto task = scheduled;
        scheduled = nullptr;
        if (task) { task->Run(); task->Dispose(); }
    }
    void Fixture(Actor* at) {
        memset(at, 0, sizeof(PlayerCharacter));
        actor = at;
        *reinterpret_cast<void***>(at) = actorTable;
        *reinterpret_cast<void***>(&at->actorValueOwner) = avTable;
        actorTable[0xE2] = reinterpret_cast<void*>(&Combat);
        actorTable[0x115] = reinterpret_cast<void*>(&Sentient);
        avTable[1] = reinterpret_cast<void*>(&Current);
        avTable[2] = reinterpret_cast<void*>(&WrongMaximum);
        at->formType = kFormType_Character;
        at->formID = 0x14;
        at->handleRefObject.m_uiRefCount = 1;
        at->loadedState = reinterpret_cast<TESObjectREFR::LoadedState*>(loadedBytes);
        at->loadedState->node = reinterpret_cast<NiNode*>(nodeBytes);
        at->processManager = reinterpret_cast<ActorProcessManager*>(processBytes);
        *reinterpret_cast<void**>(processBytes + 4) = middleBytes;
        *reinterpret_cast<void**>(processBytes + 8) = highBytes;
        *reinterpret_cast<void**>(middleBytes + 0xB4) = nodeBytes;
        *reinterpret_cast<void**>(middleBytes + 0xB8) = nodeBytes;
        reinterpret_cast<NiAVObject*>(nodeBytes)->m_worldTransform.pos = NiPoint3(10, 20, 30);
        *reinterpret_cast<float*>(highBytes + 0x10) = 5.f;
        *g_thePlayer = reinterpret_cast<PlayerCharacter*>(at);
    }
}
namespace TrueHUDLE {
    void Log(const char*, ...) {}
    int Fault(const char* context, EXCEPTION_POINTERS*) {
        printf("guarded test fault: %s\n", context); ++faults; return EXCEPTION_EXECUTE_HANDLER;
    }
    bool Disabled() { return faults >= 3; }
}
#include "UILifecycle.inl"

namespace {
    unsigned notificationOriginalCalls = 0, notificationABIFailures = 0;
    const char* notificationName = nullptr;
    SInt32 notificationCount = 0;
    UInt32 notificationAdded = 0;
    void __cdecl OriginalNotification(const char* name,SInt32 count,UInt32 added) {
        if (name!=notificationName || count!=notificationCount || added!=notificationAdded) ++notificationABIFailures;
        ++notificationOriginalCalls;
    }
    __declspec(naked) void NotificationReturn() {
        __asm {
            add esp,12
            cmp ebx,dword ptr [ebp+8]
            jne BadNotificationRegisters
            cmp esi,0x11223344
            jne BadNotificationRegisters
            cmp edi,0x55667788
            je GoodNotificationRegisters
        BadNotificationRegisters:
            inc notificationABIFailures
        GoodNotificationRegisters:
            pop edi
            pop esi
            pop ebx
            pop ebp
            ret
        }
    }
    __declspec(naked) void __cdecl NotificationCaller(TESForm*,const char*,SInt32,UInt32) {
        __asm {
            push ebp
            mov ebp,esp
            push ebx
            push esi
            push edi
            mov ebx,dword ptr [ebp+8]
            mov esi,0x11223344
            mov edi,0x55667788
            push dword ptr [ebp+20]
            push dword ptr [ebp+16]
            push dword ptr [ebp+12]
            mov eax,0x00861100
            // Reach the CALL instruction as part of the caller, not through
            // another CALL that would insert a nonexistent fourth stack word.
            jmp eax
        }
    }
    void EmitNotification(TESForm* form,const char* name,SInt32 count,UInt32 added) {
        notificationName=name; notificationCount=count; notificationAdded=added;
        NotificationCaller(form,name,count,added);
    }
    void TestLootNotifications() {
        using namespace TrueHUDLE;
        auto site=reinterpret_cast<unsigned char*>(0x00861100);
        site[0]=0xE8;
        const UInt32 displacement=static_cast<UInt32>(reinterpret_cast<uintptr_t>(&OriginalNotification)-0x00861105);
        memcpy(site+1,&displacement,4); site[5]=0xE9;
        const UInt32 continuation=static_cast<UInt32>(reinterpret_cast<uintptr_t>(&NotificationReturn)-0x0086110A);
        memcpy(site+6,&continuation,4);
        Check(InstallLootNotifications() && InstallLootNotifications(), "loot formatter call installer preserves existing E8 hook and is idempotent");
        alignas(16) unsigned char bytes[sizeof(TESObjectWEAP)] = {};
        auto weapon=reinterpret_cast<TESObjectWEAP*>(bytes); weapon->formType=kFormType_Weapon;
        SetSetting("bEnableRecentLoot:General",1); SetSetting("bRecentLootHideVanillaMessage:RecentLoot",1);
        SetLootReplacementReady(false); const unsigned before=notificationOriginalCalls;
        EmitNotification(weapon,"Test weapon",2,1);
        Check(notificationOriginalCalls==before+1 && !notificationABIFailures,
            "unready replacement preserves original notification and cdecl/register ABI");
        SetLootReplacementReady(true); EmitNotification(weapon,"Test weapon",2,1);
        Check(notificationOriginalCalls==before+1 && !notificationABIFailures,
            "ready enabled loot replacement suppresses only supported gain message");
        EmitNotification(weapon,"Test weapon",2,0);
        EmitNotification(weapon,"Test weapon",2,0xABCD0100);
        EmitNotification(weapon,"Test weapon",-1,1);
        Check(notificationOriginalCalls==before+4 && !notificationABIFailures,
            "removal, zero low-byte flag and negative count preserve every original argument bit");
        weapon->gameData.flags2=TESObjectWEAP::GameData::kFlags_NotPlayable;
        EmitNotification(weapon,"Test weapon",2,1);
        Check(notificationOriginalCalls==before+5, "unsupported nonplayable loot retains native message");
        weapon->gameData.flags2=0; SetSetting("bRecentLootHideVanillaMessage:RecentLoot",0);
        EmitNotification(weapon,"Test weapon",2,1);
        Check(notificationOriginalCalls==before+6, "MCM hide-vanilla toggle restores native notification");
        SetSetting("bRecentLootHideVanillaMessage:RecentLoot",1); SetSetting("bEnableRecentLoot:General",0);
        EmitNotification(weapon,"Test weapon",2,1);
        Check(notificationOriginalCalls==before+7, "disabled recent loot always keeps native notification");
        SetSetting("bEnableRecentLoot:General",1);
        EmitNotification(reinterpret_cast<TESForm*>(3),"Test weapon",2,1);
        Check(notificationOriginalCalls==before+8 && !notificationABIFailures && !faults,
            "invalid optional form fails open without spending callback fault budget");
        SetLootReplacementReady(false);
    }
    Actor* damageSelf = nullptr;
    Actor* damageAttacker = nullptr;
    float damageArgument = 0;
    unsigned damageOriginalCalls = 0;
    bool resetDuringDamage = false;
    void __fastcall OriginalDamage(Actor* self, void*, Actor* attacker, float amount) {
        if (self != damageSelf || attacker != damageAttacker ||
            !(amount == damageArgument || (std::isnan(amount) && std::isnan(damageArgument)))) ++wrongSelf;
        ++damageOriginalCalls;
        if (resetDuringDamage) TrueHUDLE::ResetDamageSignals(true);
    }
    void EmitDamage(uintptr_t slot, Actor* self, Actor* attacker, float amount) {
        damageSelf=self; damageAttacker=attacker; damageArgument=amount;
        using Call=void (__thiscall*)(Actor*,Actor*,float);
        (*reinterpret_cast<Call*>(slot))(self,attacker,amount);
    }
    void TestDamageSignals() {
        using namespace TrueHUDLE;
        for (uintptr_t slot : {0x010D00C8u,0x010D1AE0u,0x010D2428u})
            *reinterpret_cast<void**>(slot)=reinterpret_cast<void*>(&OriginalDamage);
        Check(InstallDamageHooks() && InstallDamageHooks(), "LE damage installer preserves existing hooks and is idempotent");
        ResetDamageSignals(true); SetSetting("bEnableFloatingText:General",1); SetSetting("bEnableFloatingCombatText:FloatingText",1);
        DamageEvent events[MaxFloating];
        const unsigned before=damageOriginalCalls;
        EmitDamage(0x010D2428,actor,actor,-12.5f);
        Check(damageOriginalCalls==before+1 && !wrongSelf && DrainDamageSignals(events,MaxFloating)==1 &&
            events[0].handle==7 && events[0].amount==12.5f && events[0].color==0xFF0000,
            "player damage hook chains once with both ABI arguments and copies red scalar event");
        memcpy(secondPlayerBytes,actor,sizeof(PlayerCharacter));
        auto npc=reinterpret_cast<Actor*>(secondPlayerBytes); npc->formID=0xFF000001;
        EmitDamage(0x010D1AE0,npc,actor,-3.25f);
        Check(DrainDamageSignals(events,MaxFloating)==1 && events[0].handle==8 && events[0].amount==3.25f &&
            events[0].color==0xFFFFFF, "player-inflicted NPC damage records actual native amount, not sampled health difference");
        EmitDamage(0x010D00C8,npc,npc,-4);
        EmitDamage(0x010D1AE0,npc,actor,5);
        EmitDamage(0x010D2428,actor,npc,std::numeric_limits<float>::quiet_NaN());
        Check(!DrainDamageSignals(events,MaxFloating) && damageOriginalCalls==before+5,
            "foreign damage, healing and nonfinite values are not displayed but originals still execute");
        for (unsigned i=0;i<MaxFloating+5;++i) EmitDamage(0x010D1AE0,npc,actor,-float(i+1));
        Check(DrainDamageSignals(events,MaxFloating)==MaxFloating && events[0].amount==6 &&
            events[MaxFloating-1].amount==MaxFloating+5 && !DrainDamageSignals(events,MaxFloating),
            "damage ring retains newest bounded events in chronological order and drains once");
        resetDuringDamage=true; EmitDamage(0x010D2428,actor,npc,-1); resetDuringDamage=false;
        Check(!DrainDamageSignals(events,MaxFloating), "load during chained original rejects captured prior-generation damage");
        EmitDamage(0x010D2428,actor,npc,-1); ResetDamageSignals(false);
        EmitDamage(0x010D2428,actor,npc,-1);
        Check(!DrainDamageSignals(events,MaxFloating), "load clears queued damage and rejects callbacks while disabled");
        ResetDamageSignals(true); SetSetting("bEnableFloatingText:General",0);
        EmitDamage(0x010D2428,actor,npc,-1);
        Check(!DrainDamageSignals(events,MaxFloating), "disabled floating text produces no queued damage");
        SetSetting("bEnableFloatingCombatText:FloatingText",0); ResetDamageSignals(false);
    }
    void TestLootIcons() {
        using namespace TrueHUDLE;
        LootSample output; LootTraits traits;
        traits.type=kFormType_Armor; traits.weight=0; traits.parts=(1u<<2)|(1u<<3);
        ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"lightarmor_body") && output.iconColor==0x756000,
            "armor multi-slot precedence selects body before hands");
        traits.weight=1; traits.parts=1u<<9; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"armor_shield") && output.iconColor==0x6B7585, "heavy shield uses original label and palette");
        traits.weight=2; traits.parts=1u<<6; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"armor_ring"), "untyped ring slot infers jewelry");
        traits.parts=1u<<12; traits.keywords=Clothing|Jewelry; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"default_armor"), "explicit clothing keyword takes precedence over jewelry inference");
        traits.keywords=0; traits.parts=1u<<3; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"clothing_hands"), "unarmored hand slot infers clothing");
        traits.type=kFormType_Book; traits.subtype=0xFF; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"book_note"), "LE note subtype selects note icon");
        traits.subtype=0; traits.keywords=Recipe|SpellTome; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"book_note"), "recipe keyword precedes spell-tome keyword");
        traits.keywords=SpellTome; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"book_tome"), "spell tome icon");
        traits.type=kFormType_Weapon; traits.subtype=6; traits.keywords=Warhammer; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"weapon_hammer") && output.iconColor==0xA4A5BF, "warhammer keyword distinguishes shared two-hand axe type");
        traits.formID=0xE3C16; ClassifyLootTraits(traits,output); Check(!strcmp(output.icon,"weapon_pickaxe"), "Skyrim pickaxe identity override");
        traits.formID=0x010E3C16; ClassifyLootTraits(traits,output); Check(!strcmp(output.icon,"weapon_hammer"), "another plugin local ID cannot impersonate vanilla pickaxe");
        traits.formID=0; traits.type=kFormType_Ammo; traits.flags=TESAmmo::kNotBolt; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"weapon_arrow"), "LE ammo nonbolt flag selects arrow");
        traits.flags=0; ClassifyLootTraits(traits,output); Check(!strcmp(output.icon,"weapon_bolt"), "LE bolt icon");
        traits.type=kFormType_Misc; traits.keywords=0; traits.formID=0xF; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"misc_gold") && output.iconColor==0xCCCC33, "Skyrim gold uses coin label and color");
        traits.formID=0x0100000F; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"default_misc"), "mod local gold-number collision remains ordinary misc item");
        traits.formID=0; traits.keywords=ChildClothes|Gem; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"clothing_body"), "children clothing keyword precedes gem keyword");
        traits.type=kFormType_Potion; traits.flags=AlchemyItem::kFlag_Food|AlchemyItem::kFlag_Poison;
        traits.useSound=0xB6435; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"food_wine"), "food and potion-use sound take precedence over poison flag");
        traits.flags=AlchemyItem::kFlag_Poison; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"potion_poison") && output.iconColor==0xAD00B3, "poison palette");
        traits.flags=0; const UInt32 avs[]={24,27,155,25,28,156,26,29,157,41,42,43};
        const char* labels[]={"potion_health","potion_health","potion_health","potion_magic","potion_magic","potion_magic",
            "potion_stam","potion_stam","potion_stam","potion_fire","potion_shock","potion_frost"};
        bool actorValues=true;
        for (unsigned i=0;i<12;++i) { traits.primaryAV=avs[i]; ClassifyLootTraits(traits,output); actorValues &= !strcmp(output.icon,labels[i]); }
        Check(actorValues, "all potion AV groups match verified LE registration indices");
        traits.type=kFormType_SoulGem; traits.capacity=2; traits.soul=1; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"soulgem_partial") && output.iconColor==0xC0BAFF, "partial lesser soul gem palette");
        traits.capacity=5; traits.soul=5; ClassifyLootTraits(traits,output);
        Check(!strcmp(output.icon,"soulgem_grandfull"), "full grand soul gem icon");
        traits.formID=0x63B29; ClassifyLootTraits(traits,output); Check(!strcmp(output.icon,"soulgem_azura"), "Black Star identity override");
        alignas(16) unsigned char bytes[sizeof(TESObjectWEAP)] = {};
        auto weapon=reinterpret_cast<TESObjectWEAP*>(bytes); weapon->formType=kFormType_Weapon; weapon->gameData.type=1;
        Check(ClassifyLoot(weapon,output) && !strcmp(output.icon,"weapon_sword"), "production LE weapon field reader feeds icon policy");
        weapon->flags=4;
        Check(ClassifyLoot(weapon,output), "LE weapon overrides generic record playability flag with its own data flags");
        weapon->gameData.flags2=TESObjectWEAP::GameData::kFlags_NotPlayable;
        Check(!ClassifyLoot(weapon,output), "unplayable LE weapon filtered before UI publication");
        memset(bytes,0,sizeof(bytes)); auto armor=reinterpret_cast<TESObjectARMO*>(bytes);
        armor->formType=kFormType_Armor; armor->flags=4;
        Check(!ClassifyLoot(armor,output), "generic LE nonplayable record flag filters hidden armor");
        memset(bytes,0,sizeof(bytes)); auto book=reinterpret_cast<TESObjectBOOK*>(bytes);
        book->formType=kFormType_Book; book->data.flags=TESObjectBOOK::Data::kType_CantBeTaken;
        Check(ClassifyLoot(book,output), "book takeability is distinct from native generic playability");
        memset(bytes,0,sizeof(bytes)); auto alchemy=reinterpret_cast<AlchemyItem*>(bytes); alchemy->formType=kFormType_Potion;
        Check(ClassifyLoot(alchemy,output) && !strcmp(output.icon,"default_potion"), "empty LE potion effect list fails safely to generic potion");
        alignas(16) unsigned char effectBytes[sizeof(EffectSetting)] = {};
        auto effect=reinterpret_cast<EffectSetting*>(effectBytes); effect->properties.primaryValue=25; alchemy->unk44=effect;
        Check(ClassifyLoot(alchemy,output) && !strcmp(output.icon,"potion_magic"), "LE cached effect setting supplies primary AV without an engine call");
        alchemy->unk44=nullptr; MagicItem::EffectItem first; first.mgef=effect; MagicItem::EffectItem* entries[]={&first};
        alchemy->effectItemList.arr.entries=entries; alchemy->effectItemList.arr.capacity=alchemy->effectItemList.count=1;
        Check(ClassifyLoot(alchemy,output) && !strcmp(output.icon,"potion_magic"), "LE first effect fallback validates each pointer and capacity");
        alchemy->effectItemList.count=2;
        Check(ClassifyLoot(alchemy,output) && !strcmp(output.icon,"default_potion"), "invalid effect count cannot traverse beyond capacity");
    }
    struct WidgetFixture final : TRUEHUD_API::WidgetBase {
        unsigned initialized = 0, updated = 0, disposed = 0;
        bool throwUpdate = false, removeDuringUpdate = false;
        void Initialize() override { ++initialized; }
        void Update(float) override {
            ++updated;
            if (throwUpdate) throw 42;
            if (removeDuringUpdate) _widgetState = kRemoved;
        }
        void Dispose() override { ++disposed; }
    };
    struct CustomHostFixture {
        unsigned loaded = 0, registered = 0, attached = 0, detached = 0;
        bool loadOK = true, attachOK = true;
        bool Load(UInt32, const char*) { ++loaded; return loadOK; }
        void Register(UInt32, UInt32) { ++registered; }
        bool Attach(UInt32, UInt32, UInt32, const char*, TRUEHUD_API::WidgetBase&) {
            ++attached; return attachOK;
        }
        void Detach(UInt32, UInt32, UInt32) { ++detached; }
    };
    void TestPublicAPI() {
        using namespace TrueHUDLE;
        using namespace TRUEHUD_API;
        auto api = static_cast<IVTrueHUD2*>(RequestAPI(InterfaceVersion::V2));
        Check(api && RequestAPI(InterfaceVersion::V1) == api && RequestAPI(InterfaceVersion::V3) && RequestAPI(InterfaceVersion::V4) &&
            !RequestAPI(static_cast<InterfaceVersion>(255)), "public API exposes all four complete versions and rejects unknown versions");
        Check(api->RequestTargetControl(7) == APIResult::OK && api->GetTargetControlOwner() == 7 &&
            api->RequestTargetControl(8) == APIResult::AlreadyTaken && api->ReleaseTargetControl(7) == APIResult::OK,
            "public vtable target ownership maps backend results");
        Check(api->RequestSpecialResourceBarsControl(9) == APIResult::OK &&
            api->RequestSpecialResourceBarsControl(9) == APIResult::AlreadyGiven &&
            api->GetSpecialResourceBarControlOwner() == 9 && api->FlashActorSpecialBar(8, 7, false) == APIResult::NotOwner,
            "public special ownership rejects another provider");
        Check(api->ReleaseSpecialResourceBarControl(9) == APIResult::OK &&
            api->GetPlayerWidgetBarColorsControlOwner() == InvalidResourceOwner,
            "public release and upstream unused color-owner contract");
        SetAPIThread(1234); Check(api->GetTrueHUDThreadId() == 1234, "public thread ID follows actual UI publication");
        APICommand output; bool current;
        api->AddBoss(7); Check(PopAPICommand(output, current) && current && output.kind == APICommandKind::AddBoss &&
            output.actor == 7, "public actor request reaches owned UI queue");
        api->RemoveActorInfoBar(7, WidgetRemovalMode::Delayed);
        Check(PopAPICommand(output, current) && current && output.kind == APICommandKind::RemoveActor && output.value == 2,
            "public delayed actor removal preserves mode");
        api->FlashActorValue(7, 26, true);
        Check(PopAPICommand(output, current) && output.kind == APICommandKind::FlashActor && output.value == 26 && output.longFlash,
            "public flash copies actor value and duration");
        api->FlashActorValue(7, 23, true); api->AddBoss(0);
        Check(!PopAPICommand(output, current), "invalid public actor/AV requests never enter queue");
        bool rejected = false;
        api->LoadCustomWidgets(7, "../bad.swf", [&](APIResult result) { rejected = result == APIResult::WidgetFailedToLoad; });
        Check(rejected && !PopAPICommand(output, current), "unsafe custom library path rejects with a real failure callback");
        std::string path = "Test/Widget.swf";
        api->LoadCustomWidgets(7, path, {}); path[0] = 'X';
        Check(PopAPICommand(output, current) && !strcmp(output.text, "Test/Widget.swf"), "public string_view is copied before caller storage changes");
        ResetAPIColors(); Settings settings; CopySettings(settings); const double defaultColor = settings.uHealthColor;
        api->OverrideBarColor(7, 24, BarColorType::BarColor, 0x112233);
        Check(PopAPICommand(output, current) && output.kind == APICommandKind::OverrideColor, "v2 color reaches UI queue");
        ProcessAPIColor(output); const UInt32 revision = CopyAPIColors(7, settings);
        Check(revision && settings.uHealthColor == 0x112233, "v2 applies requested actor color");
        CopySettings(settings); CopyAPIColors(8, settings);
        Check(settings.uHealthColor == defaultColor, "v2 actor colors do not leak to another actor");
        api->OverrideSpecialBarColor(7, BarColorType::FlashColor, 0xABCDEF); PopAPICommand(output, current); ProcessAPIColor(output);
        CopySettings(settings); CopyAPIColors(7, settings);
        Check(settings.uSpecialFlashColor == 0xABCDEF && settings.uHealthColor == 0x112233, "v2 all twenty colors coexist without changing defaults");
        api->RevertBarColor(7, 24, BarColorType::BarColor); PopAPICommand(output, current); ProcessAPIColor(output);
        CopySettings(settings); CopyAPIColors(7, settings);
        Check(settings.uHealthColor == defaultColor && settings.uSpecialFlashColor == 0xABCDEF,
            "v2 partial revert restores only requested color");
        ResetAPIColors(); CopySettings(settings);
        Check(!CopyAPIColors(7, settings) && settings.uHealthColor == defaultColor, "load clears transient actor color overrides");
        UInt32 ids[] = {7, 8}; bool bosses[] = {false, true}; PublishInfoBars(ids, bosses, 2);
        Check(HasPublishedInfoBar(7, true) && HasPublishedInfoBar(8, false) && !HasPublishedInfoBar(8, true) &&
            !HasPublishedInfoBar(9, false), "plain info-bar publication preserves floating-only filter");
        PublishInfoBars(nullptr, nullptr, 0);
        Check(!HasPublishedInfoBar(7, false), "menu cleanup clears published info handles");
    }
    void TestCustomWidgets() {
        using namespace TrueHUDLE;
        using namespace TRUEHUD_API;
        CustomDefinitions definitions;
        CustomHostFixture host;
        CustomWidgetRegistry<CustomHostFixture> registry(host, definitions);
        APICommand command; command.kind = APICommandKind::LoadCustom; command.owner = 7;
        strcpy_s(command.text, "Test.swf"); unsigned callbacks = 0;
        command.callback = [&](APIResult result) { if (result == APIResult::OK) ++callbacks; };
        registry.Process(command); registry.Process(command);
        Check(callbacks == 2 && host.loaded == 1, "same library load callback succeeds without destroying existing container");
        command.callback = {}; command.kind = APICommandKind::RegisterCustomType; command.type = 3;
        registry.Process(command); registry.Process(command);
        Check(host.registered == 1, "duplicate custom type registration does not reset live widgets");
        auto widget = std::make_shared<WidgetFixture>();
        command.kind = APICommandKind::AddCustom; command.id = 42; command.widget = widget;
        strcpy_s(command.text, "Symbol"); registry.Process(command); registry.Tick(.1f);
        Check(widget->initialized == 1 && widget->updated == 1 && widget->_widgetID == 42 &&
            widget->_widgetState == WidgetBase::kActive, "custom clip attaches and initializes before its first update");
        command.widget = widget; command.id = 43; registry.Process(command); registry.Tick(.1f);
        Check(host.attached == 1 && widget->initialized == 1, "one client widget cannot be attached to two clips");
        command.widget.reset(); command.kind = APICommandKind::RemoveCustom; command.id = 42; command.owner = 8;
        registry.Process(command); Check(widget->disposed == 0, "custom removal isolates plugin owner");
        command.owner = 7; command.value = 2; registry.Process(command);
        Check(widget->_widgetState == WidgetBase::kDelayedRemoval && widget->disposed == 0, "delayed custom removal delegates animation state to client");
        widget->removeDuringUpdate = true; registry.Tick(.1f);
        Check(widget->disposed == 1 && host.detached == 1 && !widget->_view &&
            widget->_widgetState == WidgetBase::kRemoved, "client-completed removal disposes once and clears movie ownership");
        auto throwing = std::make_shared<WidgetFixture>(); throwing->throwUpdate = true;
        command.kind = APICommandKind::AddCustom; command.id = 44; command.widget = throwing;
        registry.Process(command); registry.Tick(.1f); registry.Tick(.1f); registry.Tick(.1f); registry.Tick(.1f);
        Check(throwing->updated == 3 && throwing->disposed == 1 && host.detached == 2,
            "three client update faults retire only that custom widget");
        auto waiting = std::make_shared<WidgetFixture>(); host.attachOK = false;
        command.widget = waiting; command.id = 45; registry.Process(command);
        for (unsigned i = 0; i < 21; ++i) registry.Tick(.25f);
        Check(waiting->initialized == 0 && waiting->_widgetState == WidgetBase::kRemoved && host.detached == 2,
            "missing asynchronous symbol expires without Initialize or Dispose on unattached client");
        host.attachOK = true;
        auto cleared = std::make_shared<WidgetFixture>(); command.widget = cleared; command.id = 46;
        registry.Process(command); registry.Tick(.1f); registry.Clear(); registry.Clear();
        Check(cleared->disposed == 1 && host.detached == 3, "load cleanup is idempotent and disposes every live widget once");
        CustomHostFixture nextHost; CustomWidgetRegistry<CustomHostFixture> next(nextHost, definitions);
        next.RestoreDefinitions(); next.RestoreDefinitions();
        Check(nextHost.loaded == 1 && nextHost.registered == 1, "recreated menu restores persistent library/type definitions once");
        CustomHostFixture failedHost; failedHost.loadOK = false;
        CustomWidgetRegistry<CustomHostFixture> failed(failedHost, definitions);
        failed.RestoreDefinitions(); unsigned failuresReported = 0;
        command.kind = APICommandKind::LoadCustom; strcpy_s(command.text, "Test.swf");
        command.callback = [&](APIResult result) { if (result == APIResult::WidgetFailedToLoad) ++failuresReported; };
        failed.Process(command);
        Check(failuresReported == 1 && failedHost.loaded == 2,
            "failed restored library cannot report a cached success to a new load request");
        // The failed request cleared that library; re-establish it before the
        // replacement check against the first menu's live registry.
        command.callback = {}; registry.Process(command);
        command.widget.reset(); command.kind = APICommandKind::LoadCustom; command.callback = {};
        strcpy_s(command.text, "Replacement.swf"); registry.Process(command);
        Check(host.loaded == 3 && host.registered == 3, "replacement library restores all registered types");
    }
    void TestDebugDrawing() {
        using namespace TrueHUDLE;
        using namespace TRUEHUD_API;
        auto api = static_cast<IVTrueHUD4*>(RequestAPI(InterfaceVersion::V4));
        const NiPoint3 a(0,0,0), b(0,0,10), x(1,0,0), y(0,1,0);
        const NiQuaternion identity{1,0,0,0};
        auto take = [&] { return CopyDebugDrawing(debugFixture, MaxDebugLines, GetTickCount64()); };
        ResetDebugDrawing(); api->DrawLine(a,b,0,0x12345678,2);
        Check(take() == 1 && debugFixture[0].start.z == 0 && debugFixture[0].end.z == 10 &&
            debugFixture[0].color == 0x12345678 && debugFixture[0].thickness == 2 && take() == 0,
            "v3 line copies values and zero duration renders exactly once");
        api->DrawLine(a,b,1); Check(take() == 1 && take() == 1, "positive debug duration survives multiple frames");
        Check(CopyDebugDrawing(debugFixture, MaxDebugLines, GetTickCount64()+2000) == 0,
            "debug duration expires against its original deadline");
        api->DrawPoint(a,7,0,0x11223344);
        Check(take() == 1 && debugFixture[0].pointSize == 7 && debugFixture[0].color == 0x11223344,
            "point preserves screen-space radius and RGBA");
        api->DrawArrow(a,b,2); Check(take() == 3, "arrow includes shaft and both arrowhead edges");
        api->DrawBox(a,{2,3,4},identity); unsigned n=take(); bool bounds=n==12;
        for (unsigned i=0;i<n;++i) bounds &= std::fabs(debugFixture[i].start.x)==2 &&
            std::fabs(debugFixture[i].start.y)==3 && std::fabs(debugFixture[i].start.z)==4;
        Check(bounds, "oriented box emits twelve edges at expected extents");
        const NiQuaternion rotation{.70710678f,0,0,.70710678f};
        api->DrawBox(a,{2,3,4},rotation); n=take(); bounds=n==12;
        for (unsigned i=0;i<n;++i) bounds &= std::fabs(std::fabs(debugFixture[i].start.x)-3)<.001f &&
            std::fabs(std::fabs(debugFixture[i].start.y)-2)<.001f;
        Check(bounds, "quaternion rotation swaps box extents at a quarter turn");
        api->DrawCircle(a,x,y,2,4); n=take();
        Check(n==4 && std::fabs(debugFixture[3].end.x-2)<.001f && std::fabs(debugFixture[3].end.y)<.001f,
            "circle closes back onto its first world-space vertex");
        api->DrawHalfCircle(a,x,y,2,4); n=take();
        Check(n==4 && std::fabs(debugFixture[3].end.x+2)<.001f && debugFixture[1].end.y>1.99f,
            "half circle traverses the positive arc to opposite endpoint");
        api->DrawCylinder(a,b,2,4); n=take(); unsigned longitudinal=0;
        for (unsigned i=0;i<n;++i) if (debugFixture[i].start.z==0 && debugFixture[i].end.z==10) ++longitudinal;
        Check(n==12 && longitudinal==4, "cylinder includes two rings and longitudinal edges");
        api->DrawCone(a,b,10,1,1,4); n=take();
        const auto& tip=debugFixture[0].end;
        Check(n==8 && std::fabs(tip.z-10*std::cos(1.f))<.001f &&
            std::fabs(tip.x*tip.x+tip.y*tip.y+tip.z*tip.z-100)<.001f,
            "cone preserves upstream angular convention and direction/ray length");
        api->DrawSphere(a,2,16); n=take(); bool onSphere=n>100;
        for (unsigned i=0;i<n;++i) {
            const auto& p=debugFixture[i].start;
            onSphere &= std::fabs(p.x*p.x+p.y*p.y+p.z*p.z-4)<.001f;
        }
        Check(onSphere, "wire sphere ring vertices lie on requested sphere");
        api->DrawCapsule(a,b,2); n=take(); bool capsule=n>60; float minimum=0,maximum=10;
        for (unsigned i=0;i<n;++i) {
            minimum=std::min(minimum,debugFixture[i].start.z);
            maximum=std::max(maximum,debugFixture[i].start.z);
        }
        Check(capsule && minimum< -1.99f && maximum>11.99f, "v4 capsule extends both hemispheres beyond segment endpoints");
        api->DrawCapsule(a,5,2,identity); n=take(); minimum=0; maximum=0;
        for (unsigned i=0;i<n;++i) { minimum=std::min(minimum,debugFixture[i].start.z); maximum=std::max(maximum,debugFixture[i].start.z); }
        Check(n>60 && std::fabs(minimum+5)<.001f && std::fabs(maximum-5)<.001f,
            "v3 capsule half-height includes hemispheres");
        api->DrawLine({std::numeric_limits<float>::quiet_NaN(),0,0},b);
        api->DrawBox(a,x,{0,0,0,0}); api->DrawCircle(a,x,y,-1,4);
        Check(take()==0, "nonfinite geometry, zero quaternion and negative radius rejected");
        for (unsigned i=0;i<MaxDebugLines+10;++i) api->DrawLine(a,b);
        Check(take()==MaxDebugLines && take()==0, "debug queue is bounded and fully drains one-frame lines");
        api->DrawLine(a,b,100); ResetDebugDrawing(); Check(take()==0, "load reset removes durable debug geometry");
    }
}
void RunTests() {
    if (GetEnvironmentVariableA("TRUEHUD_HARNESS_STALL", nullptr, 0)) Sleep(INFINITE);
    Install(0x006A7320, reinterpret_cast<void*>(&Level));
    Install(0x006D4360, reinterpret_cast<void*>(&Hostile));
    Install(0x004DE820, reinterpret_cast<void*>(&Name));
    Install(0x006DE980, reinterpret_cast<void*>(&Modifier));
    Install(0x004A9180, reinterpret_cast<void*>(&Lookup));
    Install(0x0065CC00, reinterpret_cast<void*>(&CreateHandle));
    Install(0x00AB84C0, reinterpret_cast<void*>(&Projection));
    Install(0x006E3E30, reinterpret_cast<void*>(&CaptureSink));
    Install(0x00451A30, reinterpret_cast<void*>(&LookupForm));
    Install(0x00F51DAE, reinterpret_cast<void*>(&CastForm));
    Install(0x006AA140, reinterpret_cast<void*>(&Sentient));
    Install(0x0059C090, reinterpret_cast<void*>(&SoulType));
    Install(0x008C0FA0, reinterpret_cast<void*>(&FixtureLOS));
    Install(0x006E7A50, reinterpret_cast<void*>(&GetMount));
    *const_cast<UInt32*>(g_invalidRefHandle) = 0xFFFFFFFF;
    Fixture(reinterpret_cast<Actor*>(actorBytes));
    using namespace TrueHUDLE;
    ActorSample sample;
    Check(SampleActor(7, sample), "real sample body executes against fake LE ABI");
    Check(sample.current[0] == 135 && sample.maximum[0] == 150, "maximum includes temporary buffs, subtracts damage");
    Check(sample.maximum[1] == 50 && sample.maximum[2] == 100, "magicka and stamina maximum");
    Check(sample.level == 20 && !strcmp(sample.name, "LE fixture"), "copied actor identity");
    Check(sample.head.z == 30 && sample.body.x == 10 && sample.voiceRecovery == 5, "body and voice linked reads");
    Check(wrongMaximumCalls == 0 && modifierCalls == 3 && wrongSelf == 0, "correct receiver and modifier ABI");
    TestSpecialResources();
    TestWidgetDelegates();
    TestAPIQueue();
    TestPublicAPI();
    TestCustomWidgets();
    TestDebugDrawing();
    TestDamageSignals();
    TestLootIcons();
    TestLootNotifications();
    actor->actorState.flags04 = 7u << 21; actor->flags2 = 1u << 18;
    Check(SampleActor(7, sample) && sample.bleeding && sample.essential && !sample.dead && !sample.boss,
        "LE bleedout state7 and runtime essential flag are distinguished from death");
    actor->actorState.flags04 = 8u << 21; actor->flags2 = 0;
    Check(SampleActor(7, sample) && sample.bleeding && !sample.essential && !sample.dead,
        "LE bleedout state8 also recognized for a nonessential actor");
    actor->actorState.flags04 = 1u << 21;
    Check(SampleActor(7, sample) && sample.dead && !sample.bleeding, "dying state remains a dead target rather than bleedout");
    actor->actorState.flags04 = 0; actor->flags2 = 1u << 16;
    Check(SampleActor(7, sample) && !sample.essential && !sample.bleeding,
        "commanded bit16 does not masquerade as essential bit18");
    actor->flags2 = 0;
    struct LeveledFixture { UInt32 table; BSExtraData* next; TESForm* original; TESForm* templ; } leveled{
        0x01079D1C,nullptr,reinterpret_cast<TESForm*>(baseBytes),nullptr};
    BaseExtraList::PresenceBitfield presence{};
    presence.bits[0x2D/8] = 1u << (0x2D%8);
    const auto oldPresence = actor->extraData.m_presence;
    const auto oldExtraData = actor->extraData.m_data;
    actor->extraData.m_presence = &presence; actor->extraData.m_data = reinterpret_cast<BSExtraData*>(&leveled);
    auto originalForm = reinterpret_cast<TESForm*>(baseBytes);
    const UInt32 oldID=originalForm->formID; const UInt8 oldType=originalForm->formType;
    originalForm->formID=0x1234; originalForm->formType=kFormType_NPC;
    Check(OriginalBaseID(actor)==0x1234, "LE original leveled-NPC base is read from ExtraLeveledCreature+8");
    leveled.original=reinterpret_cast<TESForm*>(3);
    Check(!OriginalBaseID(actor), "invalid original-base pointer fails closed");
    leveled.table=0; leveled.next=reinterpret_cast<BSExtraData*>(&leveled);
    Check(!OriginalBaseID(actor), "cyclic optional extra-data chain is bounded");
    actor->extraData.m_presence=reinterpret_cast<BaseExtraList::PresenceBitfield*>(3);
    Check(!OriginalBaseID(actor), "invalid extra-data presence link fails before dereference");
    actor->extraData.m_presence=oldPresence; actor->extraData.m_data=oldExtraData;
    originalForm->formID=oldID; originalForm->formType=oldType;
    memcpy(secondPlayerBytes,actor,sizeof(PlayerCharacter));
    auto summon=reinterpret_cast<Actor*>(secondPlayerBytes); summon->flags2=1u<<16;
    auto commanderHandle=reinterpret_cast<UInt32*>(middleBytes+0x140); *commanderHandle=7;
    Check(IsPlayerAlly(summon) && actor->handleRefObject.GetRefCount()==1,
        "commanded actor inherits player ally status and releases commander pin");
    const UInt32 oldFlags=actor->flags1;
    *g_thePlayer=reinterpret_cast<PlayerCharacter*>(summon); actor->flags1=Actor::kFlags_IsPlayerTeammate;
    // Use the first actor as the commanded creature and the second actor as
    // teammate commander to exercise a commander that is not the player.
    actor->flags1=0; actor->flags2=1u<<16; summon->flags2=0; summon->flags1=Actor::kFlags_IsPlayerTeammate;
    *g_thePlayer=nullptr; *commanderHandle=8;
    Check(IsPlayerAlly(actor) && summon->handleRefObject.GetRefCount()==1,
        "teammate-owned summon is an ally without leaking its commander reference");
    summon->flags1=0;
    Check(!IsPlayerAlly(actor) && summon->handleRefObject.GetRefCount()==1,
        "enemy-owned summon is not a player ally merely because it is commanded");
    *commanderHandle=*g_invalidRefHandle;
    Check(!IsPlayerAlly(actor), "invalid commanding handle fails closed");
    const auto oldProcess=actor->processManager; actor->processManager=reinterpret_cast<ActorProcessManager*>(3);
    Check(!IsPlayerAlly(actor), "broken commanded-process link rejected before dereference");
    actor->processManager=oldProcess; actor->flags2=0; actor->flags1=oldFlags;
    *g_thePlayer=reinterpret_cast<PlayerCharacter*>(actor); *commanderHandle=0;
    Check(sample.soul == 3, "creature soul type uses real LE two-word cdecl contract");
    sentientSoul = true;
    Check(SampleActor(7, sample) && sample.soul == 6, "sentient AL result maps to black-soul icon");
    sentientSoul = false;
    auto originalBase = actor->baseForm;
    actor->baseForm = reinterpret_cast<TESForm*>(baseBytes);
    auto& keywordFixture = *reinterpret_cast<BGSKeywordForm*>(keywordBytes);
    auto& keywordForm = *reinterpret_cast<TESForm*>(keywordFormBytes);
    keywordFixture.keywords = keywordPointers; keywordFixture.numKeywords = 1;
    keywordPointers[0] = reinterpret_cast<BGSKeyword*>(&keywordForm);
    keywordForm.formID = 0x35D59;
    Check(SampleActor(7, sample) && sample.soul == 7, "Dragon keyword takes icon precedence");
    keywordForm.formID = 0x103AD2;
    Check(SampleActor(7, sample) && sample.soul == 0, "noSoulTrap keyword suppresses soul icon");
    keywordFixture.numKeywords = 0;
    actor->baseForm = originalBase;
    memcpy(secondPlayerBytes, actor, sizeof(PlayerCharacter));
    mountedActor = true;
    Check(SampleActor(7, sample) && sample.mounted && sample.mountStamina == 75 && sample.mountMaximumStamina == 100,
          "native owned mount reference supplies buff-aware mount stamina");
    Check(reinterpret_cast<Actor*>(secondPlayerBytes)->handleRefObject.GetRefCount() == 1,
          "mount getter reference released after sampling");
    mountSelf = true;
    Check(SampleActor(7, sample) && !sample.mounted && actor->handleRefObject.GetRefCount() == 1,
          "self-mount output rejected without reference leak or recursion");
    mountedActor = mountSelf = false;
    *g_thePlayer = reinterpret_cast<PlayerCharacter*>(secondPlayerBytes);
    *reinterpret_cast<void**>(0x01B2E8BC) = losGraphBytes;
    *reinterpret_cast<const void***>(losGraphBytes + 0xAC) = losChildren;
    *reinterpret_cast<UInt16*>(losGraphBytes + 0xB2) = 1;
    Check(SampleActor(7, sample) && sample.visible && losCalls == 1, "non-target uses verified LE LOS contract");
    hasLOS = false;
    Check(SampleActor(7, sample) && !sample.visible, "occluded actor bar is hidden");
    const unsigned beforeTarget = losCalls;
    Check(SampleActor(7, sample, true) && sample.visible && losCalls == beforeTarget, "current target bypasses LOS as upstream");
    invisibleActor = true;
    Check(SampleActor(7, sample, true) && !sample.visible && losCalls == beforeTarget, "invisibility hides even current-target bar without physics query");
    invisibleActor = false;
    *reinterpret_cast<void**>(0x01B2E8BC) = nullptr;
    Check(SampleActor(7, sample) && !sample.visible && losCalls == beforeTarget, "missing LOS graph hides bar without calling unready engine");
    Check(reinterpret_cast<Actor*>(secondPlayerBytes)->handleRefObject.GetRefCount() == 1 && actor->handleRefObject.GetRefCount() == 1,
          "LOS sampling releases both target and player pins");
    *g_thePlayer = reinterpret_cast<PlayerCharacter*>(actor);
    hasLOS = true;
    Check(actor->handleRefObject.GetRefCount() == 1, "lookup reference released");
    auto validCurrent = avTable[1];
    avTable[1] = actorBytes;
    Check(!SampleActor(7, sample) && faults == 0, "readable non-executable AV function refused before call");
    avTable[1] = validCurrent;
    auto validPrimary = *reinterpret_cast<void***>(actor);
    *reinterpret_cast<void***>(actor) = reinterpret_cast<void**>(3);
    Check(!SampleActor(7, sample) && faults == 0, "broken actor primary vtable rejected");
    *reinterpret_cast<void***>(actor) = validPrimary;
    auto loaded = actor->loadedState;
    actor->loadedState = reinterpret_cast<TESObjectREFR::LoadedState*>(3);
    Check(!SampleActor(7, sample), "unaligned loaded-state link rejected");
    actor->loadedState = loaded;
    auto middle = *reinterpret_cast<void**>(processBytes + 4);
    *reinterpret_cast<void**>(processBytes + 4) = reinterpret_cast<void*>(0x3001);
    Check(!SampleActor(7, sample), "broken middle process rejected");
    *reinterpret_cast<void**>(processBytes + 4) = middle;
    Check(actor->handleRefObject.GetRefCount() == 1, "failure paths release reference");
    Check(faults == 0, "bad pointer links rejected before spending fault budget");
    actor->formType = kFormType_Weapon;
    Check(!SampleActor(7, sample), "non-actor handle rejected");
    actor->formType = kFormType_Character;
    auto large = static_cast<Actor*>(VirtualAlloc(reinterpret_cast<void*>(0x90000000), 0x10000,
                                                 MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    Check(large == reinterpret_cast<Actor*>(0x90000000), "large-address fixture allocated");
    if (large) { Fixture(large); Check(SampleActor(7, sample), "actor above 2GB accepted"); }
    Fixture(reinterpret_cast<Actor*>(actorBytes));
    double parsed;
    Check(ParseNumber(" 2.75 ;comment", parsed) && parsed == 2.75, "dot config decimal");
    Check(ParseNumber("-2,75", parsed) && parsed == -2.75, "comma config decimal");
    Check(!ParseNumber("nan", parsed) && !ParseNumber("1.2.3", parsed), "invalid decimals rejected");
    Check(ParseNumber("0xDF2020", parsed, true) && parsed == 0xDF2020, "hex colors");
    auto region = static_cast<char*>(VirtualAlloc(nullptr, 8192, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    Check(region != nullptr, "string boundary fixture allocated");
    if (region) {
        memcpy(region + 4093, "abc", 3);
        DWORD previousProtect;
        VirtualProtect(region + 4096, 4096, PAGE_NOACCESS, &previousProtect);
        char copied[8];
        CopyString(region + 4093, copied, sizeof(copied));
        Check(!strcmp(copied, "abc") && faults == 0, "string copied once per readable region, stops before protected page");
        CopyString(nullptr, copied, sizeof(copied));
        Check(copied[0] == 0, "missing string becomes empty");
        VirtualFree(region, 0, MEM_RELEASE);
    }
    Check(!SetSetting("missing:General", 1), "unknown setting refused");
    auto nativeCharges = reinterpret_cast<float*>(0x012B2F90);
    nativeCharges[0] = 25; nativeCharges[1] = 75;
    *reinterpret_cast<UInt8*>(0x01B3EA69) = 1;
    ChargeSample charges[2]; ReadNativeCharges(charges);
    Check(charges[0].show && charges[0].percent == 25 && charges[1].show && charges[1].percent == 75,
          "native charge reader preserves left and right hand percentages");
    *reinterpret_cast<UInt8*>(0x01B3EA69) = 0;
    nativeCharges[1] = (std::numeric_limits<float>::max)();
    ReadNativeCharges(charges);
    Check(!charges[0].show && !charges[1].show && charges[1].percent == 0,
          "disabled left hand and native FLT_MAX absent-enchantment sentinel hide meters");
    nativeCharges[1] = -1; ReadNativeCharges(charges);
    Check(!charges[1].show, "invalid native charge percentage is not passed to Flash");
    float cooldown = 0;
    Check(ShoutProgress(30, cooldown, 3) == 0, "new shout cooldown starts at zero progress");
    Check(std::abs(ShoutProgress(3, cooldown, 3) - .8f) < 1e-5f &&
          std::abs(ShoutProgress(1.5f, cooldown, 3) - .9f) < 1e-5f,
          "LE shout finish interval uses native80/20 progression");
    Check(ShoutProgress(0, cooldown, 3) == 1 && cooldown == 0, "ready shout clears transient cooldown");
    ShoutProgress(20, cooldown, 0);
    Check(std::isfinite(ShoutProgress(10, cooldown, 0)), "zero custom finish time avoids division by zero");
    auto hud = reinterpret_cast<HUDMenu*>(hudBytes);
    auto enemy = reinterpret_cast<EnemyHealth*>(enemyBytes);
    *reinterpret_cast<UInt32*>(enemy) = 0x010E51BC;
    enemy->handle = 17;
    hudParts[0] = reinterpret_cast<HUDObject*>(3); hudParts[1] = enemy;
    hud->hudComponents.arr.entries = hudParts;
    hud->hudComponents.arr.capacity = hud->hudComponents.count = 2;
    UInt32 nativeHandle = 999;
    Check(ReadNativeTarget(hud, nativeHandle) && nativeHandle == 17,
          "actual native HUD traversal skips invalid components and copies enemy handle");
    hud->hudComponents.count = 3;
    Check(!ReadNativeTarget(hud, nativeHandle) && nativeHandle == 0,
          "HUD count beyond capacity rejects without stale handle");
    hud->hudComponents.count = 2;
    *reinterpret_cast<UInt32*>(enemy) = 0x010E51C0;
    Check(!ReadNativeTarget(hud, nativeHandle), "different HUD object is not mistaken for EnemyHealth");
    Check(!ReadNativeTarget(nullptr, nativeHandle), "absent native menu allows safe crosshair fallback");
    auto names = reinterpret_cast<UIStringHolder*>(stringHolderBytes);
    *reinterpret_cast<UIStringHolder**>(0x012E35E0) = names;
    names->hudMenu.data = "HUD Menu";
    auto message = reinterpret_cast<UIMessage*>(messageBytes);
    message->strData.data = names->hudMenu.data;
    message->objData = reinterpret_cast<IUIMessageData*>(hudDataBytes);
    *reinterpret_cast<UInt32*>(hudDataBytes + 8) = 0xB;
    *reinterpret_cast<UInt32*>(hudDataBytes + 0x24) = 25;
    HUDSignal decoded;
    Check(DecodeHUDSignal(message, decoded) && decoded.kind == HUDSignal::Flash && decoded.value == 1,
          "LE HUD flash payload decodes magicka at verified offsets");
    message->strData.data = "Other Menu";
    Check(!DecodeHUDSignal(message, decoded) && decoded.kind == HUDSignal::None, "foreign menu cannot inject HUD signal");
    message->strData.data = names->hudMenu.data;
    message->message = 3;
    Check(!DecodeHUDSignal(message, decoded), "close event is not interpreted as HUD data");
    message->message = 0;
    Install(0x00863B40, reinterpret_cast<void*>(&OriginalHUDMessage));
    *reinterpret_cast<void**>(0x010D450C) = reinterpret_cast<void*>(&OriginalSprint);
    *reinterpret_cast<UInt32*>(0x010E538C) = 0x00863B40;
    *reinterpret_cast<void**>(0x010E5390) = reinterpret_cast<void*>(&OriginalHUDFrame);
    Check(InstallHUDHooks() && InstallHUDHooks(), "actual vtable installer chains existing hook and is idempotent");
    using HUDFrameCall = void (__thiscall*)(IMenu*, UInt32, UInt32);
    (*reinterpret_cast<HUDFrameCall*>(0x010E5390))(reinterpret_cast<IMenu*>(hudBytes), 0x3C888889, 0x55667788);
    Check(originalAdvanceCalls == 1 && !wrongSelf,
        "native HUD frame hook preserves original receiver, interval bits, time and exactly one call");
    using HUDMessageCall = UInt32 (__thiscall*)(IMenu*, UIMessage*);
    auto hook = *reinterpret_cast<HUDMessageCall*>(0x010E538C);
    Check(hook(reinterpret_cast<IMenu*>(hudBytes), message) == 0x1234 && originalMessageCalls == 1,
          "HUD hook preserves original receiver, argument, return and exactly one call");
    HUDSignals hudSignals;
    CopyHUDSignals(hudSignals);
    Check(hudSignals.flashes[1] == 1, "HUD callback publishes scalar flash signal");
    *reinterpret_cast<UInt32*>(hudDataBytes + 8) = 0x17;
    *reinterpret_cast<const char**>(hudDataBytes + 0x0C) = "CartMode";
    hudDataBytes[0x20] = 1;
    hook(reinterpret_cast<IMenu*>(hudBytes), message); CopyHUDSignals(hudSignals);
    Check(hudSignals.cartMode && originalMessageCalls == 2, "cart-mode signal copied without swallowing native message");
    ResetHUDSignals(); CopyHUDSignals(hudSignals);
    Check(!hudSignals.cartMode && hudSignals.flashes[1] == 0, "load reset clears flash and cart transients");
    Check(DistanceScale(50, true, 1) == 100 && DistanceScale(9000, true, 1) == 50,
          "near and far actor scales clamp to original range");
    Check(DistanceScale(1250, true, 2) == 150 && DistanceScale(9000, false, 2) == 200,
          "distance scaling midpoint and MCM disable preserve configured multiplier");
    auto matrix = reinterpret_cast<float*>(0x01B3EA10);
    matrix[12] = 2; matrix[13] = 3; matrix[14] = 4; matrix[15] = 5;
    float px, py, pz, cameraDistance;
    Project(NiPoint3(10, 20, 30), px, py, pz, &cameraDistance);
    Check(cameraDistance == 205, "camera distance uses homogeneous matrix denominator, not normalized depth");
    projectionNaN = true;
    Check(!Project(NiPoint3(10, 20, 30), px, py, pz) && px == 0 && py == 0 && pz == 0,
          "non-finite native projection cannot publish NaN Flash coordinates");
    projectionNaN = false; projectionReject = true;
    Check(!Project(NiPoint3(10, 20, 30), px, py, pz) && px == 0 && py == 0 && pz == 0,
          "native rejected projection clears all output coordinates");
    projectionReject = false;
    NumberFixture::Movie oldHUD{80}, newHUD{65};
    HUDNumberOverride<NumberFixture> hudOverride("fixture");
    hudOverride.Apply(&oldHUD, true, 0);
    hudOverride.Apply(&oldHUD, true, 0);
    hudOverride.Apply(&oldHUD, false, 0);
    Check(oldHUD.number == 80, "repeated HUD hiding preserves and restores original alpha");
    hudOverride.Apply(&oldHUD, true, -50, true);
    hudOverride.Apply(&oldHUD, true, -100, true);
    Check(oldHUD.number == -20, "subtitle offsets derive from original position without cumulative drift");
    const unsigned oldWrites = oldHUD.writes;
    hudOverride.Apply(&newHUD, true, 0);
    hudOverride.Restore(&newHUD);
    Check(newHUD.number == 65 && oldHUD.writes == oldWrites, "replacement HUD restored without touching expired movie");
    hudOverride.Apply(&newHUD, true, 0);
    hudOverride.Restore(nullptr);
    Check(newHUD.number == 0, "destroyed movie identity forgotten without dereference");
    newHUD.number = 1;
    HUDNumberOverride<NumberFixture> visibilityOverride("fixture-visible");
    visibilityOverride.Apply(&newHUD, true, 0, false, false);
    newHUD.number = 1; // another HUD producer changes visibility
    visibilityOverride.Apply(&newHUD, true, 0, false, false);
    Check(newHUD.number == 1, "force-hide disabled allows subsequent native visibility changes");
    visibilityOverride.Apply(&newHUD, true, 0, false, true);
    Check(newHUD.number == 0, "force-hide enabled reapplies configured visibility");
    visibilityOverride.Restore(&newHUD);
    Check(newHUD.number == 1, "force-hide removal restores saved visibility");
    SetSetting("uBossBarMaxCount:BossBars", 999);
    Check(GetSetting("uBossBarMaxCount:BossBars") == 5, "boss count clamped");
    SetSetting("fInfoBarOpacity:ActorInfoBars", -99);
    Check(GetSetting("fInfoBarOpacity:ActorInfoBars") == 0.01, "opacity clamped to upstream schema minimum");
    Check(setlocale(LC_ALL, "French_France.1252") != nullptr, "French CRT locale active for parser regression");
    Check(ParseNumber("0.75", parsed) && parsed == .75 && ParseNumber("0,75", parsed) && parsed == .75,
          "configuration decimals independent of French process locale");
    setlocale(LC_ALL, "C");
    auto data = reinterpret_cast<DataHandler*>(dataBytes);
    auto mod = reinterpret_cast<ModInfo*>(modBytes);
    strcpy_s(mod->name, "Skyrim.esm"); mod->modIndex = 0;
    data->modList.loadedModCount = 1; data->modList.loadedMods[0] = mod;
    *reinterpret_cast<DataHandler**>(0x012E2CB8) = data;
    InitializeBossRules();
    auto base = reinterpret_cast<TESForm*>(baseBytes);
    auto race = reinterpret_cast<TESRace*>(raceBytes);
    actor->baseForm = base; actor->race = race;
    base->formID = 0x900; race->formID = 0x12E82;
    Check(IsBoss(actor), "original race boss rule");
    base->formID = 0x801;
    Check(!IsBoss(actor), "NPC blacklist overrides race recognition");
    base->formID = 0x800; race->formID = 0x999;
    Check(!IsBoss(actor), "RemoveNPC repeat-key override applied");
    auto originalNPC=reinterpret_cast<TESForm*>(lootFormBytes);
    const auto savedOriginalID=originalNPC->formID; const auto savedOriginalType=originalNPC->formType;
    originalNPC->formID=0x802; originalNPC->formType=kFormType_NPC;
    leveled.table=0x01079D1C; leveled.next=nullptr; leveled.original=originalNPC;
    actor->extraData.m_presence=&presence; actor->extraData.m_data=reinterpret_cast<BSExtraData*>(&leveled);
    base->formID=0x900;
    Check(IsBoss(actor), "original leveled-NPC base matches configured boss even when current base does not");
    originalNPC->formID=0x801; race->formID=0x12E82; base->formID=0x802;
    Check(!IsBoss(actor), "original-base blacklist overrides both current NPC and race boss rules");
    actor->extraData.m_presence=oldPresence; actor->extraData.m_data=oldExtraData;
    originalNPC->formID=savedOriginalID; originalNPC->formType=savedOriginalType;
    base->formID=0x800; race->formID=0x999;
    auto location = reinterpret_cast<BGSLocation*>(locationBytes);
    location->formType = kFormType_Location;
    auto refType = reinterpret_cast<TESForm*>(refTypeBytes);
    refType->formID = 0x130F7;
    locationRefs[0].type = refType; locationRefs[0].id = actor->formID;
    *reinterpret_cast<void**>(locationBytes + 0x40) = locationRefs;
    *reinterpret_cast<UInt32*>(locationBytes + 0x48) = 1;
    *reinterpret_cast<void**>(reinterpret_cast<char*>(actor) + 0x6B0) = location;
    PrepareLocationBosses(actor);
    Check(IsBoss(actor), "native location special-reference boss recognition");
    *reinterpret_cast<UInt32*>(locationBytes + 0x48) = 0xFFFFFFFF;
    PrepareLocationBosses(actor);
    Check(!IsBoss(actor), "invalid location array rejected and previous answer cleared");
    *reinterpret_cast<void**>(reinterpret_cast<char*>(actor) + 0x6B0) = nullptr;
    SKSETaskInterface tasks{};
    tasks.AddTask = Schedule;
    SKSEMessagingInterface messages{};
    messages.GetEventDispatcher = Dispatcher;
    InitializeState(&tasks);
    RegisterEvents(&messages);
    Check(capturedSinks == 4 && combat && hit && crosshair && container, "four actual production event sinks registered");
    ResetState(true);
    mountedActor = emptyMount = true;
    RequestFrame(); Drain();
    Check(MountExhausted(), "published mount stamina exposes only a plain exhausted flag");
    HUDSignals beforeSprint, afterSprint;
    CopyHUDSignals(beforeSprint);
    Sprint(); CopyHUDSignals(afterSprint);
    Check(originalSprintCalls == 1 && wrongSelf == 0 && afterSprint.flashes[2] == beforeSprint.flashes[2] + 1,
          "sprint hook forwards both x86 arguments once and flashes an exhausted mount");
    SetSetting("bPlayerWidgetDisplayMountStamina:PlayerWidget", 0);
    Sprint(); CopyHUDSignals(beforeSprint);
    Check(originalSprintCalls == 2 && beforeSprint.flashes[2] == afterSprint.flashes[2],
          "mount display setting disables additional sprint flash without breaking original");
    SetSetting("bPlayerWidgetDisplayMountStamina:PlayerWidget", 1);
    ResetState(true);
    Sprint(); CopyHUDSignals(afterSprint);
    Check(!MountExhausted() && originalSprintCalls == 3 && afterSprint.flashes[2] == 0,
          "load reset prevents a stale mount flash while original sprint still runs");
    mountedActor = emptyMount = false;
    RequestFrame(); Drain();
    Sprint(); CopyHUDSignals(afterSprint);
    Check(!MountExhausted() && originalSprintCalls == 4 && afterSprint.flashes[2] == 0,
          "an unmounted ready player cannot inherit empty default mount stamina");
    auto lootForm = reinterpret_cast<TESForm*>(lootFormBytes);
    lootForm->formID = 0x1234; lootForm->formType = kFormType_Ingredient;
    reinterpret_cast<TESFullName*>(lootNameBytes)->name.data = "Test ingredient";
    ResetState(true);
    TESContainerChangedEvent lootEvent{};
    lootEvent.itemFormId = 0x1234; lootEvent.toFormId = 0x14; lootEvent.count = 2;
    container->ReceiveEvent(&lootEvent, nullptr);
    RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.lootCount == 1 && fixtureFrame.loot[0].count == 2 &&
          !strcmp(fixtureFrame.loot[0].name, "Test ingredient"), "loot event resolves plain name and count on sample task");
    lootEvent.fromFormId = 0x14;
    container->ReceiveEvent(&lootEvent, nullptr);
    lootEvent.fromFormId = 0; lootEvent.count = 0xFFFFFFFF;
    container->ReceiveEvent(&lootEvent, nullptr);
    container->ReceiveEvent(reinterpret_cast<TESContainerChangedEvent*>(4), nullptr);
    RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.lootCount == 1 && faults == 0, "inventory self-transfers, negative counts and invalid event pointers rejected");
    lootEvent.count = 1;
    for (unsigned i = 0; i < MaxLoot + 5; ++i) container->ReceiveEvent(&lootEvent, nullptr);
    RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.lootCount == MaxLoot && fixtureFrame.loot[0].serial == 7 &&
          fixtureFrame.loot[MaxLoot - 1].serial == MaxLoot + 6, "loot ring keeps newest entries in chronological order");
    ResetState(true); RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.lootCount == 0, "save/load reset clears transient loot events");
    auto otherActor = reinterpret_cast<Actor*>(secondPlayerBytes);
    otherActor->formID = 0xFF000001;
    ResetState(true);
    TESCombatEvent combatEvent{};
    combatEvent.source = otherActor; combatEvent.target = otherActor;
    combat->ReceiveEvent(&combatEvent, nullptr);
    RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.count == 0, "unrelated NPC combat does not populate HUD tracking");
    combatEvent.source=actor; combatEvent.target=otherActor;
    combat->ReceiveEvent(&combatEvent,nullptr); RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.count==1 && fixtureFrame.actors[0].handle==8,
        "player-initiated combat tracks the enemy target as well as the allied source");
    ResetState(true); combatEvent.source=otherActor;
    combatEvent.target = actor;
    combat->ReceiveEvent(&combatEvent, nullptr);
    RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.count == 1 && fixtureFrame.actors[0].handle == 8, "enemy fighting player is tracked");
    SetSetting("bEnableFloatingText:General",1); SetSetting("bEnableFloatingCombatText:FloatingText",1);
    EmitDamage(0x010D1AE0,otherActor,actor,-25);
    RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.floatingCount==1 && fixtureFrame.floating[0].amount==25 &&
        fixtureFrame.floating[0].position.z==30 && fixtureFrame.floating[0].color==0xFFFFFF,
        "actual damage callback crosses game task into a plain world-text snapshot");
    RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(!fixtureFrame.floatingCount, "damage world-text snapshot is not replayed on next game sample");
    SetSetting("bEnableFloatingText:General",0); SetSetting("bEnableFloatingCombatText:FloatingText",0);
    ResetState(true); ApplyActorCommand(8, true, false, 0);
    RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.count == 1 && fixtureFrame.actors[0].boss && fixtureFrame.actors[0].manualRole == 2 &&
        fixtureFrame.actors[0].removal == 255, "API boss role survives game-task scalar publication");
    ApplyActorCommand(8, true, true, 2); RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.count == 1 && fixtureFrame.actors[0].removal == 2, "API delayed actor removal reaches next UI snapshot");
    ApplyActorCommand(8, false, false, 0); RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    Check(fixtureFrame.count == 1 && !fixtureFrame.actors[0].boss && fixtureFrame.actors[0].manualRole == 1 &&
        fixtureFrame.actors[0].removal == 255, "API re-add cancels pending removal and explicitly selects floating role");
    ResetState(true);
    bulkHandles = true;
    for (UInt32 id = 100; id < 100 + MaxActors; ++id) SetNativeTarget(id, true);
    SetNativeTarget(999, true);
    RequestFrame(); Drain(); CopyFrame(fixtureFrame);
    bool currentFound = false, oldestFound = false;
    for (UInt32 i = 0; i < fixtureFrame.count; ++i) {
        currentFound |= fixtureFrame.actors[i].handle == 999;
        oldestFound |= fixtureFrame.actors[i].handle == 100;
    }
    Check(fixtureFrame.count == MaxActors && currentFound && !oldestFound,
          "full tracker admits current target by replacing oldest unrelated actor");
    bulkHandles = false;
    ResetState(true);
    SKSECrosshairRefEvent event(actor);
    crosshair->ReceiveEvent(&event, nullptr);
    RequestFrame(); RequestFrame();
    Check(scheduled != nullptr, "UI request schedules sample task once");
    Drain();
    auto& frame = fixtureFrame;
    CopyFrame(frame);
    Check(frame.ready && frame.target == 7 && frame.player.maximum[0] == 150, "production frame publishes values");
    SetNativeTarget(0, true);
    crosshair->ReceiveEvent(&event, nullptr);
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(frame.target == 0, "available native target has priority over crosshair events");
    SetNativeTarget(7, true);
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(frame.target == 7, "native enemy handle reaches game snapshot");
    SetNativeTarget(0, false);
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(frame.target == 7, "missing native component falls back to current crosshair");
    Check(RequestTargetControlNative(InvalidResourceOwner) == ResourceResult::InvalidArgument &&
          RequestTargetControlNative(7) == ResourceResult::OK &&
          RequestTargetControlNative(7) == ResourceResult::AlreadyGiven &&
          RequestTargetControlNative(8) == ResourceResult::AlreadyTaken,
          "target ownership rejects invalid owner and competing claims");
    Check(SetControlledTarget(8, 8, false) == ResourceResult::NotOwner &&
          SetControlledTarget(7, 8, false) == ResourceResult::OK,
          "only target owner can change hard target");
    SetNativeTarget(7, true); crosshair->ReceiveEvent(&event, nullptr);
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(frame.target == 8, "native and crosshair updates cannot override owned hard target");
    SetControlledTarget(7, 7, true);
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(frame.target == 8, "owned hard target takes precedence over soft target");
    SetControlledTarget(7, *g_invalidRefHandle, false);
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(frame.target == 7, "cleared hard target falls back to owned soft target");
    Check(ReleaseTargetControlNative(8) == ResourceResult::NotOwner && ReadTargetControlOwner() == 7,
          "another client cannot release target ownership");
    ResetState(false); ResetState(true); SetNativeTarget(7, true);
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(ReadTargetControlOwner() == 7 && frame.target == 0,
          "load reset preserves ownership while clearing both transient controlled targets");
    SetControlledTarget(7, 0, false);
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(frame.target == 0, "clearing hard target after load cannot resurrect the old soft target");
    Check(ReleaseTargetControlNative(7) == ResourceResult::OK && ReadTargetControlOwner() == InvalidResourceOwner,
          "owner releases target control");
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(frame.target == 7, "release restores the latest native target");
    RequestTargetControlNative(7);
    SetNativeTarget(0, false);
    SKSECrosshairRefEvent otherCrosshair(otherActor);
    crosshair->ReceiveEvent(&otherCrosshair, nullptr);
    ReleaseTargetControlNative(7);
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(frame.target == 8, "release restores the latest crosshair when native component is absent");
    crosshair->ReceiveEvent(&event, nullptr);
    RequestFrame(); Drain(); CopyFrame(frame);
    const UInt32 oldGeneration = frame.generation;
    RequestFrame();
    ResetState(false);
    Drain(); CopyFrame(frame);
    Check(!frame.ready && frame.target == 0 && frame.generation > oldGeneration, "load reset invalidates pending target/frame");
    ResetState(true);
    resetDuringSample = true;
    RequestFrame(); Drain(); CopyFrame(frame);
    Check(!frame.ready, "a reset during engine reads prevents publication of the old generation");
    ResetState(true);
    const int baseline = actor->handleRefObject.GetRefCount();
    const auto start = std::chrono::steady_clock::now();
    for (unsigned i = 0; i < 2000; ++i) { RequestFrame(); Drain(); }
    const double micros = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start).count() / 2000;
    Check(actor->handleRefObject.GetRefCount() == baseline, "2000 frame samples do not leak actor references");
    *g_thePlayer = reinterpret_cast<PlayerCharacter*>(secondPlayerBytes);
    *reinterpret_cast<void**>(0x01B2E8BC) = losGraphBytes;
    losFault = true;
    Check(!SampleActor(7, sample), "LOS engine fault is contained");
    Check(actor->handleRefObject.GetRefCount() == baseline &&
          reinterpret_cast<Actor*>(secondPlayerBytes)->handleRefObject.GetRefCount() == 1,
          "SEH finally releases target and player references after LOS fault");
    losFault = false;
    *g_thePlayer = reinterpret_cast<PlayerCharacter*>(actor);
    mountedActor = mountFault = true;
    Check(!SampleActor(7, sample) && reinterpret_cast<Actor*>(secondPlayerBytes)->handleRefObject.GetRefCount() == 1,
          "mount reference acquired before native fault is released by finally");
    mountedActor = mountFault = false;
    TestRealUILifecycle();
    forceFault = true;
    Check(!SampleActor(7, sample), "third independent engine fault contained");
    RequestFrame();
    Check(faults == 3 && !scheduled, "sample callback stops scheduling after three faults");
    printf("%d checks, %d failures; sample-only %.2f us/tick; Frame=%zu bytes; faults=%d\n",
           checks, failures, micros, sizeof(Frame), faults);
}
int main() {
    if (GetEnvironmentVariableA("FAKEGAME_CHILD", nullptr, 0)) {
        lateSinkAddress = &lateSinkStorage.Get();
        if (&lateSinkStorage.Get() != lateSinkAddress || lateSinkAddress->Receive() != 73) return 2;
    }
    return FakeGame::Run(nullptr, RunTests);
}
