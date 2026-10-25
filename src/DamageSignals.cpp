#include "DamageSignals.h"
#include "Settings.h"
#include "skse/GameAPI.h"
#include <cmath>

namespace TrueHUDLE {
    namespace {
        SRWLOCK damageLock = SRWLOCK_INIT;
        DamageEvent events[MaxFloating];
        unsigned head = 0, count = 0;
        UInt32 generation = 1, serial = 0;
        bool running = false;
        volatile LONG callbackThread = 0;
        using HealthDamage = void (__thiscall*)(Actor*, Actor*, float);
        HealthDamage originalActor = nullptr, originalCharacter = nullptr, originalPlayer = nullptr;
        bool Capture(Actor* self, Actor* attacker, float amount, DamageEvent& event) {
            if (Disabled() || !std::isfinite(amount) || amount >= 0 || amount < -1e9f ||
                !GetSetting("bEnableFloatingText:General") || !GetSetting("bEnableFloatingCombatText:FloatingText")) return false;
            const auto player = *g_thePlayer;
            if (!Readable(player, sizeof(Actor)) || (self != player && attacker != player) ||
                !Readable(self, sizeof(Actor)) || self->formType != kFormType_Character) return false;
            const UInt32 handle = self->CreateRefHandle();
            if (!handle || handle == *g_invalidRefHandle) return false;
            event.handle = handle; event.amount = -amount; event.color = self == player ? 0xFF0000 : 0xFFFFFF;
            AcquireSRWLockShared(&damageLock);
            event.generation = generation; const bool active = running;
            ReleaseSRWLockShared(&damageLock); return active;
        }
        void Publish(DamageEvent event) {
            const LONG thread = static_cast<LONG>(GetCurrentThreadId());
            if (!InterlockedCompareExchange(&callbackThread, thread, 0)) Log("health damage callback thread=%lu", thread);
            AcquireSRWLockExclusive(&damageLock);
            if (running && event.generation == generation) {
                if (!++serial) ++serial;
                event.serial = serial;
                events[(head+count)%MaxFloating] = event;
                if (count == MaxFloating) head = (head+1)%MaxFloating; else ++count;
            }
            ReleaseSRWLockExclusive(&damageLock);
        }
        void Handle(HealthDamage original, Actor* self, Actor* attacker, float amount) {
            DamageEvent event; bool captured = false;
            // Self belongs to the engine call until chaining begins. Copy only
            // its handle before the original may kill/dispose it; never read
            // self/attacker after returning from the original.
            __try { captured = Capture(self,attacker,amount,event); }
            __except(Fault("health damage capture",GetExceptionInformation())) {}
            __try { original(self,attacker,amount); }
            __except(Fault("health damage original",GetExceptionInformation())) { return; }
            if (captured && !Disabled()) Publish(event);
        }
        void __fastcall ActorDamage(Actor* self, void*, Actor* attacker, float amount) { Handle(originalActor,self,attacker,amount); }
        void __fastcall CharacterDamage(Actor* self, void*, Actor* attacker, float amount) { Handle(originalCharacter,self,attacker,amount); }
        void __fastcall PlayerDamage(Actor* self, void*, Actor* attacker, float amount) { Handle(originalPlayer,self,attacker,amount); }
        bool Install(uintptr_t address, HealthDamage& original, void* replacement) {
            if (original) return true;
            auto slot = reinterpret_cast<void**>(address);
            if (!Readable(slot,sizeof(void*)) || !Executable(*slot)) return false;
            DWORD protection;
            if (!VirtualProtect(slot,sizeof(void*),PAGE_READWRITE,&protection)) return false;
            void* previous;
            do {
                previous = *slot;
                if (!Executable(previous)) {
                    DWORD ignored; VirtualProtect(slot,sizeof(void*),protection,&ignored); return false;
                }
                original = reinterpret_cast<HealthDamage>(previous);
            } while (InterlockedCompareExchangePointer(slot,replacement,previous) != previous);
            DWORD ignored; const bool restored = VirtualProtect(slot,sizeof(void*),protection,&ignored) != 0;
            Log("health damage slot=%p chain=%p protection-restored=%u",slot,previous,restored); return true;
        }
    }
    bool InstallDamageHooks() {
        // LE slot103/byte40C, not SE slot104. Native caller006E0877..8A
        // supplies ECX Actor, attacker then float bits;006E4700 RET8, void.
        // Constructors prove each primary table; see evidence/*damage-caller.
        return Install(0x010D00C8,originalActor,reinterpret_cast<void*>(&ActorDamage)) &&
            Install(0x010D1AE0,originalCharacter,reinterpret_cast<void*>(&CharacterDamage)) &&
            Install(0x010D2428,originalPlayer,reinterpret_cast<void*>(&PlayerDamage));
    }
    void ResetDamageSignals(bool enabled) {
        AcquireSRWLockExclusive(&damageLock);
        if (!++generation) ++generation;
        running = enabled; head = count = 0;
        ReleaseSRWLockExclusive(&damageLock);
    }
    unsigned DrainDamageSignals(DamageEvent* output, unsigned capacity) {
        if (!output || !capacity) return 0;
        AcquireSRWLockExclusive(&damageLock);
        const unsigned copied = count < capacity ? count : capacity;
        for (unsigned i=0;i<copied;++i) output[i] = events[(head+i)%MaxFloating];
        head = (head+copied)%MaxFloating; count -= copied;
        ReleaseSRWLockExclusive(&damageLock); return copied;
    }
}
