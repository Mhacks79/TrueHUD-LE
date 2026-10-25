#include "Runtime.h"
#include "SpecialResources.h"
#include "skse/GameAPI.h"
#include "skse/NiNodes.h"
#include "skse/GameRTTI.h"
#include "skse/GameData.h"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace TrueHUDLE
{
    bool IsPlayerAlly(Actor* actor) {
        TESObjectREFR* commander = nullptr;
        bool ally = false;
        __try {
            __try {
                if (!Readable(actor,sizeof(Actor)) || actor->formType != kFormType_Character) __leave;
                if (actor == *g_thePlayer || (actor->flags1 & Actor::kFlags_IsPlayerTeammate)) { ally = true; __leave; }
                if (!(actor->flags2 & (1u << 16))) __leave;
                // LE Actor006A8EC0 -> AIProcess00720E90; native caller006D4120
                // only follows this handle for commanded actors. No SE offset.
                const char* process = reinterpret_cast<const char*>(actor->processManager);
                if (!Readable(process,0x0C)) __leave;
                const char* middle = *reinterpret_cast<const char* const*>(process+4);
                if (!Readable(middle,0x144)) __leave;
                UInt32 handle = *reinterpret_cast<const UInt32*>(middle+0x140);
                if (!handle || handle == *g_invalidRefHandle || !LookupREFRByHandle(&handle,&commander) ||
                    !Readable(commander,sizeof(Actor)) || commander->formType != kFormType_Character) __leave;
                ally = commander == *g_thePlayer ||
                    (static_cast<Actor*>(commander)->flags1 & Actor::kFlags_IsPlayerTeammate) != 0;
            }
            __finally { if (commander) commander->handleRefObject.DecRefHandle(); }
        }
        __except(Fault("commanded ally",GetExceptionInformation())) { ally = false; }
        return ally;
    }
    void ReadNativeCharges(ChargeSample output[2]) {
        // Published native HUD values: same readers as HUDChargeMeter::Update.
        const float* values = reinterpret_cast<const float*>(0x012B2F90);
        const bool leftEnabled = *reinterpret_cast<const UInt8*>(0x01B3EA69) != 0;
        for (unsigned i = 0; i < 2; ++i) {
            const float value = values[i];
            const bool valid = std::isfinite(value) && value >= 0 && value <= 100;
            output[i].percent = valid ? value : 0;
            output[i].show = valid && (i != 0 || leftEnabled);
        }
    }
    bool Executable(const void* pointer) {
        MEMORY_BASIC_INFORMATION memory;
        return VirtualQuery(pointer, &memory, sizeof(memory)) && memory.State == MEM_COMMIT &&
            !(memory.Protect & (PAGE_GUARD | PAGE_NOACCESS)) &&
            (memory.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY));
    }
    // Caller owns UI-thread access to the menu. Publish only the scalar handle.
    bool ReadNativeTarget(const HUDMenu* hud, UInt32& handle) {
        handle = 0;
        if (!Readable(hud, sizeof(HUDMenu))) return false;
        const auto& parts = hud->hudComponents;
        if (parts.count > 128 || parts.count > parts.arr.capacity ||
            !Readable(parts.arr.entries, parts.count * sizeof(HUDObject*))) return false;
        for (UInt32 i = 0; i < parts.count; ++i) {
            auto part = parts.arr.entries[i];
            if (Readable(part, sizeof(EnemyHealth)) &&
                *reinterpret_cast<const UInt32*>(part) == 0x010E51BC) {
                handle = static_cast<const EnemyHealth*>(part)->handle;
                return true;
            }
        }
        return false;
    }
    namespace {
        bool MountStamina(Actor* mount, float& current, float& maximum) {
            if (!Readable(mount, sizeof(Actor)) || mount->formType != kFormType_Character) return false;
            auto table = *reinterpret_cast<void***>(&mount->actorValueOwner);
            if (!Readable(table, 4 * sizeof(void*)) || !Executable(table[1])) return false;
            if (mount->processManager) {
                const char* process = reinterpret_cast<const char*>(mount->processManager);
                if (!Readable(process, 0x34)) return false;
                const void* cache = *reinterpret_cast<const void* const*>(process + 0x30);
                if (cache && !Readable(cache, 0x30)) return false;
            }
            typedef float (__thiscall* Modifier)(Actor*, UInt32, UInt32);
            current = mount->actorValueOwner.GetCurrent(26);
            maximum = current - reinterpret_cast<Modifier>(0x006DE980)(mount, 2, 26);
            return std::isfinite(current) && std::isfinite(maximum);
        }
        bool LOSViewReady() {
            const char* graph = *reinterpret_cast<const char* const*>(0x01B2E8BC);
            if (!Readable(graph, 0xBC) || !*reinterpret_cast<const UInt16*>(graph + 0xB2)) return false;
            const void* const* children = *reinterpret_cast<const void* const* const*>(graph + 0xAC);
            return Readable(children, sizeof(void*)) && Readable(*children, 0x50);
        }
        bool HasKeywordID(TESForm* form, UInt32 id) {
            if (!Readable(form, sizeof(TESForm))) return false;
            auto keywords = DYNAMIC_CAST(form, TESForm, BGSKeywordForm);
            if (!Readable(keywords, sizeof(BGSKeywordForm)) || keywords->numKeywords > 4096 ||
                !Readable(keywords->keywords, keywords->numKeywords * sizeof(BGSKeyword*))) return false;
            for (UInt32 i = 0; i < keywords->numKeywords; ++i) {
                auto keyword = keywords->keywords[i];
                if (Readable(keyword, sizeof(TESForm)) && keyword->formID == id) return true;
            }
            return false;
        }
        UInt32 SoulIcon(Actor* actor, UInt16 level) {
            if (HasKeywordID(actor->baseForm, 0x35D59) || HasKeywordID(actor->race, 0x35D59)) return 7;
            if (HasKeywordID(actor->baseForm, 0x103AD2) || HasKeywordID(actor->race, 0x103AD2)) return 0;
            // Caller006E8BD8 consumes AL; upper return bytes are not a bool.
            typedef UInt8 (__thiscall* Sentient)(Actor*);
            if (reinterpret_cast<Sentient>(0x006AA140)(actor)) return 6;
            // Caller006E8BEC pushes two words, then ADD ESP,8; full EAX enum.
            typedef UInt32 (__cdecl* SoulType)(UInt32, UInt32);
            const UInt32 type = reinterpret_cast<SoulType>(0x0059C090)(level, 0);
            return type >= 1 && type <= 5 ? type : 0;
        }
    }
    void CopyString(const char* source, char* output, size_t capacity) {
        if (!capacity) return;
        output[0] = 0;
        size_t written = 0;
        while (written + 1 < capacity) {
            const uintptr_t address = reinterpret_cast<uintptr_t>(source);
            MEMORY_BASIC_INFORMATION region;
            if (address < 0x10000 || address >= 0xFFFF0000u ||
                !VirtualQuery(source, &region, sizeof(region)) || region.State != MEM_COMMIT ||
                (region.Protect & (PAGE_NOACCESS | PAGE_GUARD)) ||
                !(region.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY |
                                   PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) break;
            const uintptr_t end = reinterpret_cast<uintptr_t>(region.BaseAddress) + region.RegionSize;
            if (end <= address) break;
            const size_t available = std::min(size_t(end - address), capacity - written - 1);
            for (size_t i = 0; i < available; ++i) {
                output[written++] = *source++;
                if (!output[written - 1]) return;
            }
        }
        output[written] = 0;
    }
    bool Readable(const void* pointer, size_t length, bool object)
    {
        const uintptr_t start = reinterpret_cast<uintptr_t>(pointer);
        if (!length || start < 0x10000 || start >= 0xFFFF0000u ||
            length > 0xFFFF0000u - start || (object && (start & 3))) return false;
        uintptr_t cursor = start;
        const uintptr_t end = start + length;
        while (cursor < end) {
            MEMORY_BASIC_INFORMATION region;
            if (!VirtualQuery(reinterpret_cast<const void*>(cursor), &region, sizeof(region)) ||
                region.State != MEM_COMMIT || (region.Protect & (PAGE_GUARD | PAGE_NOACCESS)) ||
                !(region.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY |
                                   PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) return false;
            const uintptr_t next = reinterpret_cast<uintptr_t>(region.BaseAddress) + region.RegionSize;
            if (next <= cursor) return false;
            cursor = next;
        }
        return true;
    }

    // The LE SDK handle lookup acquires BSHandleRefObject; release even on SEH.
    // No actor/scene pointer is published to the UI consumer.
    bool SampleActor(UInt32 handle, ActorSample& sample, bool target)
    {
        TESObjectREFR* ref = nullptr;
        TESObjectREFR* playerRef = nullptr;
        Actor* mountRef = nullptr;
        bool result = false;
        sample = ActorSample{};
        if (!handle || handle == *g_invalidRefHandle) return false;
        __try {
            __try {
                if (!LookupREFRByHandle(&handle, &ref) || !Readable(ref, sizeof(Actor)) ||
                    ref->formType != kFormType_Character) __leave;
                Actor* actor = static_cast<Actor*>(ref);
                if (!Readable(actor->loadedState, sizeof(TESObjectREFR::LoadedState)) ||
                    !Readable(actor->loadedState->node, sizeof(void*) * 2)) __leave;
                const auto table = *reinterpret_cast<void***>(&actor->actorValueOwner);
                const auto primaryTable = *reinterpret_cast<void***>(actor);
                if (!Readable(table, 4 * sizeof(void*)) || !Executable(table[1]) ||
                    !Readable(primaryTable, 0x458) || !Executable(primaryTable[0xE2]) ||
                    !Executable(primaryTable[0x115])) __leave;
                sample.handle = handle;
                sample.formID = actor->formID;
                sample.position = actor->pos;
                sample.head = sample.body = actor->pos;
                sample.level = CALL_MEMBER_FN(actor, GetLevel)();
                sample.teammate = actor != *g_thePlayer && IsPlayerAlly(actor);
                const UInt32 lifeState = (actor->actorState.flags04 >> 21) & 15;
                sample.dead = lifeState == 1 || lifeState == 2;
                // Papyrus native registration008E2AE0 identifies008DA810:
                // life states7 and8 both mean bleedout. Actor IsEssential
                // native008DCB30 reads flags2+138 bit18, not base-NPC flags.
                sample.bleeding = lifeState == 7 || lifeState == 8;
                sample.essential = (actor->flags2 & (1u << 18)) != 0;
                // All links validated before calling cached AV methods below.
                void* process = actor->processManager;
                if (process) {
                    if (!Readable(process, 0x34)) __leave;
                    void* middle = *reinterpret_cast<void**>(static_cast<char*>(process) + 4);
                    void* high = *reinterpret_cast<void**>(static_cast<char*>(process) + 8);
                    void* avCache = *reinterpret_cast<void**>(static_cast<char*>(process) + 0x30);
                    if (avCache && !Readable(avCache, 0x30)) __leave;
                    if (middle) {
                        if (!Readable(middle, 0xBC)) __leave;
                        for (unsigned part = 0; part < 2; ++part) {
                            auto node = *reinterpret_cast<NiAVObject**>(static_cast<char*>(middle) + 0xB4 + part * 4);
                            if (node && Readable(node, sizeof(NiAVObject))) {
                                const auto& position = node->m_worldTransform.pos;
                                if (std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z))
                                    (part ? sample.head : sample.body) = position;
                            }
                        }
                    }
                    if (high) {
                        if (!Readable(high, 0x14)) __leave;
                        sample.voiceRecovery = *reinterpret_cast<float*>(static_cast<char*>(high) + 0x10);
                        if (!std::isfinite(sample.voiceRecovery)) __leave;
                    }
                }
                sample.combat = actor->IsInCombat();
                Actor* player = *g_thePlayer;
                if (actor != player) {
                    if (!Readable(player, sizeof(PlayerCharacter))) __leave;
                    UInt32 playerHandle = player->CreateRefHandle();
                    if (!LookupREFRByHandle(&playerHandle, &playerRef) || playerRef != player) __leave;
                }
                if (actor == player) {
                    PrepareLocationBosses(actor);
                    // Actor::GetMount: native output owns a handle reference.
                    // Caller006C2F0D and release006C32B0..D4 prove ownership.
                    typedef bool (__thiscall* GetMount)(Actor*, Actor**);
                    if (reinterpret_cast<GetMount>(0x006E7A50)(actor, &mountRef) && mountRef != actor)
                        sample.mounted = MountStamina(mountRef, sample.mountStamina, sample.mountMaximumStamina);
                }
                sample.hostile = Readable(player, sizeof(Actor)) && actor != player &&
                    CALL_MEMBER_FN(actor, IsHostileToActor)(player);
                sample.boss = sample.hostile && !sample.dead && !(sample.bleeding && sample.essential) && IsBoss(actor);
                if (actor != player && !sample.dead && !sample.boss) {
                    // Actor-value registration005ADE78/7D: Invisibility=0x36.
                    const float invisibility = actor->actorValueOwner.GetCurrent(0x36);
                    if (!std::isfinite(invisibility)) __leave;
                    sample.visible = invisibility <= 0;
                    if (sample.visible && !target) {
                        // Papyrus HasLOS008DB78D pushes3words, ADD ESP,0C;
                        // AL result. Player path aligns its own physics locals,
                        // bhkWorld::PickObject owns its read guard internally.
                        UInt8 detail = 0;
                        sample.visible = LOSViewReady() && ::HasLOS(player, actor, &detail);
                    }
                }
                sample.soul = SoulIcon(actor, sample.level);
                const char* name = CALL_MEMBER_FN(actor, GetReferenceName)();
                CopyString(name, sample.name, sizeof(sample.name));
                for (UInt32 i = 0; i < 3; ++i) {
                    sample.current[i] = actor->actorValueOwner.GetCurrent(24 + i);
                    // The vanilla LE HUD uses current - damage modifier, including
                    // temporary buffs: evidence/0085F0B0.{c,asm}, call 0085F113.
                    // SDK GetMaximum is permanent-only (006DFA90 -> 006DED10).
                    typedef float (__thiscall* Modifier)(Actor*, UInt32, UInt32);
                    sample.maximum[i] = sample.current[i] -
                        reinterpret_cast<Modifier>(0x006DE980)(actor, 2, 24 + i);
                    if (!std::isfinite(sample.current[i]) || !std::isfinite(sample.maximum[i])) __leave;
                }
                if (!std::isfinite(sample.position.x) || !std::isfinite(sample.position.y) ||
                    !std::isfinite(sample.position.z)) __leave;
                SampleSpecialResource(actor, actor == player, sample.special);
                result = true;
            }
            __finally {
                __try { if (playerRef) playerRef->handleRefObject.DecRefHandle(); }
                __finally {
                    __try { if (mountRef) mountRef->handleRefObject.DecRefHandle(); }
                    __finally { if (ref) ref->handleRefObject.DecRefHandle(); }
                }
            }
        }
        __except(Fault("SampleActor", GetExceptionInformation())) { result = false; }
        if (!result) sample = ActorSample{};
        return result;
    }

    bool Project(const NiPoint3& world, float& x, float& y, float& depth, float* cameraDistance)
    {
        // Ghidra evidence/WorldToScreen.c; actual caller proof is tracked in STATUS.
        typedef bool (__cdecl* Function)(const float*, const NiRect<float>*, const NiPoint3*,
                                         float*, float*, float*, float);
        x = y = depth = 0;
        const float* matrix = reinterpret_cast<const float*>(0x01B3EA10);
        // Same homogeneous denominator used by LE WorldPtToScreenPt3 before
        // reciprocal division, not the normalized depth passed to Flash.
        if (cameraDistance) *cameraDistance = std::abs(matrix[12] * world.x + matrix[13] * world.y +
            matrix[14] * world.z + matrix[15]);
        const bool result = reinterpret_cast<Function>(0x00AB84C0)(
            matrix,
            reinterpret_cast<const NiRect<float>*>(0x01B3EA74), &world, &x, &y, &depth, 1e-5f);
        const bool valid = result && std::isfinite(x) && std::isfinite(y) && std::isfinite(depth);
        if (!valid) x = y = depth = 0;
        return valid;
    }
}
