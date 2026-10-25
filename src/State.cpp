#include "Runtime.h"
#include "skse/GameAPI.h"
#include "skse/GameThreads.h"
#include "skse/PapyrusEvents.h"
#include "skse/GameRTTI.h"
#include "Settings.h"
#include "HUDSignals.h"
#include "TargetControl.h"
#include "APICommands.h"
#include "APIUIState.h"
#include "DebugDraw.h"
#include "DamageSignals.h"
#include "LootIcons.h"
#include "LootNotifications.h"
#include "ProcessLifetime.h"
#include "Timing.h"
#include <cstring>

namespace TrueHUDLE {
    namespace {
        SRWLOCK stateLock = SRWLOCK_INIT;
        Frame published;
        Frame samplingScratch; // one static delegate in flight; no 54KB per-call stack frame
        struct Tracked { UInt32 handle = 0; bool hit = false; ULONGLONG serial = 0; UInt8 role = 0, removal = 255; };
        ULONGLONG trackSerial = 0;
        UInt32 generation = 1, target = 0;
        UInt32 crosshairTarget = 0;
        UInt32 nativeTarget = 0;
        UInt32 targetOwner = InvalidResourceOwner, controlledHard = 0, controlledSoft = 0;
        bool nativeTargetAvailable = false;
        Tracked handles[MaxActors];
        struct LootEvent { UInt32 serial = 0, id = 0, count = 0; };
        LootEvent loot[MaxLoot];
        UInt32 lootSerial = 0;
        unsigned lootHead = 0, lootCount = 0;
        bool running = false;
        volatile LONG inFlight = 0;
        SKSETaskInterface* taskInterface = nullptr;
        DWORD samplingThread = 0;
        unsigned sampleTraceCount = 0;
        bool lastSampleReady = false;
        Timing sampleTiming{"sampling"}; // phase0: complete sample task, including engine work
        VisibilityState visibility[MaxActors]; // sampler-owned scalars, never engine pointers
        UInt32 visibilityGeneration = 0;
        LootSample sampledLoot[MaxLoot];

        UInt32 Handle(TESObjectREFR* ref) {
            if (!Readable(ref, sizeof(TESObjectREFR)) || ref->formType != kFormType_Character) return 0;
            return ref->CreateRefHandle();
        }
        bool PlayerAlly(TESObjectREFR* ref) {
            return Readable(ref, sizeof(Actor)) && ref->formType == kFormType_Character &&
                IsPlayerAlly(static_cast<Actor*>(ref));
        }
        void Track(UInt32 handle, bool hit = false) {
            if (!handle || handle == *g_invalidRefHandle) return;
            AcquireSRWLockExclusive(&stateLock);
            if (running) {
                unsigned free = MaxActors;
                bool found = false;
                for (unsigned i = 0; i < MaxActors; ++i) {
                    if (handles[i].handle == handle) { handles[i].hit |= hit; handles[i].serial = ++trackSerial; found = true; break; }
                    if (!handles[i].handle && free == MaxActors) free = i;
                }
                if (!found && free == MaxActors && (handle == target || hit)) {
                    ULONGLONG oldest = ~ULONGLONG(0);
                    for (unsigned i = 0; i < MaxActors; ++i) {
                        if (handles[i].handle != target && handles[i].serial < oldest) {
                            oldest = handles[i].serial; free = i;
                        }
                    }
                }
                if (!found && free < MaxActors) handles[free] = {handle, hit, ++trackSerial};
            }
            ReleaseSRWLockExclusive(&stateLock);
        }
        void ProjectSample(ActorSample& sample, const Settings& settings) {
            NiPoint3 point = settings.uInfoBarAnchor ? sample.head : sample.body;
            point.z += static_cast<float>(settings.fInfoBarOffsetZ);
            sample.projected = Project(point, sample.screenX, sample.screenY, sample.depth, &sample.cameraDistance);
        }
        void SampleFrameBody() {
            if (Disabled()) return;
            if (!samplingThread) {
                samplingThread = GetCurrentThreadId();
                Log("game sample task thread=%lu", samplingThread);
            }
            auto& next = samplingScratch;
            memset(&next, 0, sizeof(next)); // plain-value snapshots, no owning members
            Tracked local[MaxActors];
            LootEvent localLoot[MaxLoot];
            unsigned localLootCount;
            bool active;
            AcquireSRWLockShared(&stateLock);
            next.generation = generation;
            next.target = target;
            active = running;
            memcpy(local, handles, sizeof(local));
            localLootCount = lootCount;
            for (unsigned i = 0; i < localLootCount; ++i)
                localLoot[i] = loot[((lootCount == MaxLoot ? lootHead : 0) + i) % MaxLoot];
            ReleaseSRWLockShared(&stateLock);
            Settings settings;
            CopySettings(settings); // one coherent configuration snapshot for the whole batch
            const ULONGLONG now = GetTickCount64();
            if (visibilityGeneration != next.generation) {
                memset(visibility, 0, sizeof(visibility));
                memset(sampledLoot, 0, sizeof(sampledLoot));
                visibilityGeneration = next.generation;
            }
            Actor* player = *g_thePlayer;
            const UInt32 playerHandle = active ? Handle(player) : 0;
            next.ready = active && SampleActor(playerHandle, next.player);
            if (next.ready) {
                for (unsigned i = 0; i < MaxActors; ++i) {
                    if (!local[i].handle || local[i].handle == playerHandle) continue;
                    if (!settings.bEnableActorInfoBars && !settings.bEnableBossBars) continue;
                    ActorSample sample;
                    const bool valid = SampleActor(local[i].handle, sample, local[i].handle == next.target,
                        &visibility[i], now, static_cast<unsigned>(settings.uVisibilityUpdateIntervalMS));
                    if (valid && (sample.combat || sample.dead || sample.bleeding || local[i].role || local[i].handle == next.target)) {
                        sample.hit = local[i].hit;
                        sample.manualRole = local[i].role;
                        sample.removal = local[i].role ? local[i].removal : 255;
                        if (local[i].role) sample.boss = local[i].role == 2;
                        ProjectSample(sample, settings);
                        next.actors[next.count++] = sample;
                    } else {
                        visibility[i] = {};
                        AcquireSRWLockExclusive(&stateLock);
                        if (generation == next.generation && handles[i].handle == local[i].handle) handles[i] = {};
                        ReleaseSRWLockExclusive(&stateLock);
                    }
                }
                for (unsigned i = 0; settings.bEnableRecentLoot && i < localLootCount; ++i) {
                    const unsigned slot = localLoot[i].serial % MaxLoot;
                    if (sampledLoot[slot].serial == localLoot[i].serial) {
                        next.loot[next.lootCount++] = sampledLoot[slot];
                        continue;
                    }
                    TESForm* form = LookupFormByID(localLoot[i].id);
                    if (!Readable(form, sizeof(TESForm))) continue;
                    TESFullName* fullName = DYNAMIC_CAST(form, TESForm, TESFullName);
                    if (!Readable(fullName, sizeof(TESFullName)) || !fullName->name.data) continue;
                    auto& item = next.loot[next.lootCount];
                    if (!ClassifyLoot(form,item)) continue;
                    ++next.lootCount;
                    item.serial = localLoot[i].serial; item.formID = form->formID;
                    item.count = localLoot[i].count; item.type = form->formType;
                    CopyString(fullName->name.data, item.name, sizeof(item.name));
                    sampledLoot[slot] = item; // event snapshot; no retained form/name pointers
                }
                DamageEvent damage[MaxFloating];
                const unsigned damageCount = DrainDamageSignals(damage,MaxFloating);
                for (unsigned i=0;i<damageCount;++i) {
                    ActorSample sampled; const ActorSample* position = nullptr;
                    if (damage[i].handle == playerHandle) position = &next.player;
                    for (unsigned j=0; !position && j<next.count; ++j)
                        if (next.actors[j].handle == damage[i].handle) position = &next.actors[j];
                    if (!position && SampleActor(damage[i].handle,sampled,true)) position = &sampled;
                    if (!position) continue;
                    auto& text = next.floating[next.floatingCount++];
                    text.serial = damage[i].serial; text.amount = damage[i].amount;
                    text.color = damage[i].color; text.position = position->head;
                }
            }
            AcquireSRWLockExclusive(&stateLock);
            if (generation == next.generation) published = next;
            ReleaseSRWLockExclusive(&stateLock);
            if (sampleTraceCount < 16 && (sampleTraceCount < 3 || lastSampleReady != next.ready)) {
                ++sampleTraceCount; lastSampleReady = next.ready;
                Log("sample completed: probe=%u active=%u generation=%u player-handle=%08X ready=%u actors=%u target=%08X",
                    sampleTraceCount, active, next.generation, playerHandle, next.ready, next.count, next.target);
            }
        }
        class SampleTask final : public TaskDelegate {
        public:
            void Run() override {
                __try {
                    const ULONGLONG start = sampleTiming.Begin(); SampleFrameBody(); sampleTiming.End(0, start, 0, true);
                }
                __except(Fault("game sample task", GetExceptionInformation())) { ResetState(false); }
            }
            void Dispose() override { InterlockedExchange(&inFlight, 0); }
        } sampleTask;

        class CombatSink final : public BSTEventSink<TESCombatEvent> {
        public:
            EventResult ReceiveEvent(TESCombatEvent* event, EventDispatcher<TESCombatEvent>*) override {
                __try {
                    if (!Disabled() && Readable(event, sizeof(TESCombatEvent))) {
                        if (PlayerAlly(event->target) || PlayerAlly(event->source)) {
                            Track(Handle(event->source));
                            Track(Handle(event->target));
                        }
                    }
                }
                __except(Fault("combat event", GetExceptionInformation())) {}
                return kEvent_Continue;
            }
        };
        ProcessLifetime<CombatSink> combatSink;
        class HitSink final : public BSTEventSink<TESHitEvent> {
        public:
            EventResult ReceiveEvent(TESHitEvent* event, EventDispatcher<TESHitEvent>*) override {
                __try {
                    if (event && !Disabled() && event->target != event->caster &&
                        (PlayerAlly(event->target) || PlayerAlly(event->caster))) {
                        Track(Handle(event->target), true); Track(Handle(event->caster), true);
                    }
                }
                __except(Fault("hit event", GetExceptionInformation())) {}
                return kEvent_Continue;
            }
        };
        ProcessLifetime<HitSink> hitSink;
        class LootSink final : public BSTEventSink<TESContainerChangedEvent> {
        public:
            EventResult ReceiveEvent(TESContainerChangedEvent* event, EventDispatcher<TESContainerChangedEvent>*) override {
                __try {
                    if (!Disabled() && Readable(event, sizeof(TESContainerChangedEvent))) {
                        // Read callback-owned memory before taking our lock.
                        const UInt32 to = event->toFormId, from = event->fromFormId;
                        const UInt32 count = event->count, item = event->itemFormId;
                        if (to == 0x14 && from != 0x14 && count && count < 0x80000000u) {
                            AcquireSRWLockExclusive(&stateLock);
                            if (running) {
                                loot[lootHead] = {++lootSerial, item, count};
                                lootHead = (lootHead + 1) % MaxLoot;
                                if (lootCount < MaxLoot) ++lootCount;
                            }
                            ReleaseSRWLockExclusive(&stateLock);
                        }
                    }
                }
                __except(Fault("container event", GetExceptionInformation())) {}
                return kEvent_Continue;
            }
        };
        ProcessLifetime<LootSink> lootSink;
        class CrosshairSink final : public BSTEventSink<SKSECrosshairRefEvent> {
        public:
            EventResult ReceiveEvent(SKSECrosshairRefEvent* event, EventDispatcher<SKSECrosshairRefEvent>*) override {
                __try {
                    if (event && !Disabled()) {
                        const UInt32 value = Handle(event->crosshairRef);
                        AcquireSRWLockExclusive(&stateLock);
                        if (running) {
                            crosshairTarget = value;
                            if (targetOwner == InvalidResourceOwner && !nativeTargetAvailable) target = value;
                        }
                        ReleaseSRWLockExclusive(&stateLock);
                        Track(value);
                    }
                }
                __except(Fault("crosshair event", GetExceptionInformation())) {}
                return kEvent_Continue;
            }
        };
        ProcessLifetime<CrosshairSink> crosshairSink;
    }
    void InitializeState(SKSETaskInterface* tasks) { taskInterface = tasks; }
    void RegisterEvents(SKSEMessagingInterface* messaging) {
        g_combatEventDispatcher->AddEventSink(&combatSink.Get());
        g_hitEventDispatcher->AddEventSink(&hitSink.Get());
        g_containerChangedEventDispatcher->AddEventSink(&lootSink.Get());
        auto dispatcher = static_cast<EventDispatcher<SKSECrosshairRefEvent>*>(
            messaging->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_CrosshairEvent));
        if (dispatcher) dispatcher->AddEventSink(&crosshairSink.Get());
    }
    void RequestFrame() {
        if (taskInterface && !Disabled() && InterlockedCompareExchange(&inFlight, 1, 0) == 0)
            taskInterface->AddTask(&sampleTask);
    }
    void ForgetActor(UInt32 handle) {
        AcquireSRWLockExclusive(&stateLock);
        for (auto& tracked : handles) if (tracked.handle == handle) tracked = {};
        ReleaseSRWLockExclusive(&stateLock);
    }
    void ApplyActorCommand(UInt32 handle, bool boss, bool remove, UInt32 removalMode) {
        if (!handle || handle == *g_invalidRefHandle || removalMode > 2) return;
        Track(handle, !remove);
        AcquireSRWLockExclusive(&stateLock);
        if (running) for (auto& tracked : handles) if (tracked.handle == handle) {
            tracked.role = boss ? 2 : 1;
            tracked.removal = remove ? static_cast<UInt8>(removalMode) : 255;
            break;
        }
        ReleaseSRWLockExclusive(&stateLock);
    }
    void SetNativeTarget(UInt32 handle, bool available) {
        if (handle == *g_invalidRefHandle) handle = 0;
        AcquireSRWLockExclusive(&stateLock);
        nativeTargetAvailable = available;
        nativeTarget = available ? handle : 0;
        if (running && targetOwner == InvalidResourceOwner) target = available ? handle : crosshairTarget;
        ReleaseSRWLockExclusive(&stateLock);
        if (available) Track(handle);
    }
    ResourceResult RequestTargetControlNative(UInt32 requested) {
        if (requested == InvalidResourceOwner) return ResourceResult::InvalidArgument;
        AcquireSRWLockExclusive(&stateLock);
        const ResourceResult result = targetOwner == requested ? ResourceResult::AlreadyGiven :
            targetOwner != InvalidResourceOwner ? ResourceResult::AlreadyTaken : ResourceResult::OK;
        if (result == ResourceResult::OK) {
            targetOwner = requested; controlledHard = target; controlledSoft = 0;
        }
        ReleaseSRWLockExclusive(&stateLock);
        return result;
    }
    ResourceResult ReleaseTargetControlNative(UInt32 requested) {
        AcquireSRWLockExclusive(&stateLock);
        const bool owned = requested != InvalidResourceOwner && targetOwner == requested;
        UInt32 effective = 0;
        if (owned) {
            targetOwner = InvalidResourceOwner; controlledHard = controlledSoft = 0;
            effective = target = running ? (nativeTargetAvailable ? nativeTarget : crosshairTarget) : 0;
        }
        ReleaseSRWLockExclusive(&stateLock);
        if (effective) Track(effective);
        return owned ? ResourceResult::OK : ResourceResult::NotOwner;
    }
    ResourceResult SetControlledTarget(UInt32 requested, UInt32 handle, bool soft) {
        if (handle == *g_invalidRefHandle) handle = 0;
        AcquireSRWLockExclusive(&stateLock);
        const bool owned = requested != InvalidResourceOwner && targetOwner == requested;
        UInt32 effective = 0;
        if (owned) {
            (soft ? controlledSoft : controlledHard) = handle;
            effective = target = running ? (controlledHard ? controlledHard : controlledSoft) : 0;
        }
        ReleaseSRWLockExclusive(&stateLock);
        if (effective) Track(effective);
        return owned ? ResourceResult::OK : ResourceResult::NotOwner;
    }
    UInt32 ReadTargetControlOwner() {
        AcquireSRWLockShared(&stateLock);
        const UInt32 result = targetOwner;
        ReleaseSRWLockShared(&stateLock);
        return result;
    }
    void CopyFrame(Frame& output) {
        AcquireSRWLockShared(&stateLock);
        output = published;
        ReleaseSRWLockShared(&stateLock);
    }
    bool MountExhausted() {
        AcquireSRWLockShared(&stateLock);
        const bool exhausted = running && published.ready && published.player.mounted &&
            published.player.mountStamina <= 0;
        ReleaseSRWLockShared(&stateLock);
        return exhausted;
    }
    void ResetState(bool enabled) {
        ResetAPICommands();
        SetLootReplacementReady(false);
        ResetDamageSignals(enabled);
        ResetDebugDrawing(); PublishInfoBars(nullptr, nullptr, 0);
        AcquireSRWLockExclusive(&stateLock);
        ++generation;
        target = 0;
        crosshairTarget = 0;
        nativeTarget = 0;
        controlledHard = controlledSoft = 0; // ownership persists; transient references do not
        nativeTargetAvailable = false;
        running = enabled;
        memset(handles, 0, sizeof(handles));
        trackSerial = 0;
        memset(loot, 0, sizeof(loot)); lootCount = lootHead = 0;
        memset(&published, 0, sizeof(published));
        published.generation = generation;
        ReleaseSRWLockExclusive(&stateLock);
        ResetHUDSignals();
    }
}
