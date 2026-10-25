#include "SpecialResources.h"
#include <memory>
#include <mutex>
#include <cmath>
#include <algorithm>

namespace TrueHUDLE {
    namespace {
        struct Provider {
            ResourceCallback current, maximum;
            UInt32 owner = InvalidResourceOwner;
            bool specialMode = false, displayPlayer = true;
            volatile LONG failures = 0;
        };
        std::mutex providerMutex;
        std::shared_ptr<Provider> provider;
        UInt32 owner = InvalidResourceOwner, revision = 1;
        volatile LONG callbackThread = 0;
        void Changed() { if (!++revision) ++revision; }
        void ProviderFailure(Provider* failed, const char* reason) {
            const LONG count = InterlockedIncrement(&failed->failures);
            Log("Special resource provider=%08X failure=%ld disabled=%u reason=%s", failed->owner, count, count >= 3, reason);
            if (count == 3) {
                std::lock_guard<std::mutex> guard(providerMutex);
                if (provider.get() == failed) Changed();
            }
        }
        int ProviderException(Provider* failed, EXCEPTION_POINTERS* exception) {
            Log("Special resource exception=%08lX at=%p", exception->ExceptionRecord->ExceptionCode,
                exception->ExceptionRecord->ExceptionAddress);
            ProviderFailure(failed, "native exception");
            return EXCEPTION_EXECUTE_HANDLER;
        }
        bool InvokeCpp(Provider* active, Actor* actor, float& current, float& maximum) {
            try {
                const DWORD thread = GetCurrentThreadId();
                if (!InterlockedCompareExchange(&callbackThread, static_cast<LONG>(thread), 0))
                    Log("Special resource callback thread=%lu", thread);
                current = active->current(actor);
                maximum = active->maximum(actor);
                if (!std::isfinite(current) || !std::isfinite(maximum)) {
                    ProviderFailure(active, "non-finite result"); return false;
                }
                current = std::max(current, 0.f); maximum = std::max(maximum, 0.f);
                return true;
            } catch (...) {
                ProviderFailure(active, "C++ exception"); return false;
            }
        }
        bool InvokeGuarded(Provider* active, Actor* actor, float& current, float& maximum) {
            __try { return InvokeCpp(active, actor, current, maximum); }
            __except(ProviderException(active, GetExceptionInformation())) { return false; }
        }
    }
    ResourceResult RequestSpecialControl(UInt32 requested) {
        if (requested == InvalidResourceOwner) return ResourceResult::InvalidArgument;
        std::lock_guard<std::mutex> guard(providerMutex);
        if (owner == requested) return ResourceResult::AlreadyGiven;
        if (owner != InvalidResourceOwner) return ResourceResult::AlreadyTaken;
        owner = requested; Changed();
        return ResourceResult::OK;
    }
    ResourceResult ReleaseSpecialControl(UInt32 requested) {
        std::shared_ptr<Provider> retired;
        {
            std::lock_guard<std::mutex> guard(providerMutex);
            if (requested == InvalidResourceOwner || owner != requested) return ResourceResult::NotOwner;
            owner = InvalidResourceOwner; retired.swap(provider); Changed();
        }
        // Captured user objects may have destructors that call this API again.
        // Never destroy callbacks while holding the registry mutex.
        return ResourceResult::OK;
    }
    ResourceResult RegisterSpecialCallbacks(UInt32 requested, ResourceCallback current, ResourceCallback maximum,
                                           bool specialMode, bool displayPlayer) {
        if (requested == InvalidResourceOwner || !current || !maximum) return ResourceResult::InvalidArgument;
        auto next = std::make_shared<Provider>();
        next->current = std::move(current); next->maximum = std::move(maximum);
        next->owner = requested; next->specialMode = specialMode; next->displayPlayer = displayPlayer;
        {
            std::lock_guard<std::mutex> guard(providerMutex);
            if (owner != requested) return ResourceResult::NotOwner;
            provider.swap(next); Changed();
        }
        return ResourceResult::OK;
    }
    SpecialResourceStatus ReadSpecialStatus() {
        std::lock_guard<std::mutex> guard(providerMutex);
        SpecialResourceStatus status;
        status.owner = owner; status.revision = revision;
        status.available = provider && InterlockedCompareExchange(&provider->failures, 0, 0) < 3 && !Disabled();
        if (status.available) {
            status.specialMode = provider->specialMode; status.displayPlayer = provider->displayPlayer;
        }
        return status;
    }
    void SampleSpecialResource(Actor* actor, bool player, SpecialResourceSample& output) {
        output = {};
        std::shared_ptr<Provider> active;
        UInt32 sampledRevision;
        {
            std::lock_guard<std::mutex> guard(providerMutex);
            sampledRevision = revision; output.revision = revision;
            active = provider;
        }
        if (Disabled() || !active || !actor || (player && !active->displayPlayer) ||
            InterlockedCompareExchange(&active->failures, 0, 0) >= 3) return;
        float current = 0, maximum = 0;
        if (!InvokeGuarded(active.get(), actor, current, maximum)) return;
        {
            std::lock_guard<std::mutex> guard(providerMutex);
            if (provider != active || revision != sampledRevision || owner != active->owner ||
                InterlockedCompareExchange(&active->failures, 0, 0) >= 3) return;
            output.current = current; output.maximum = maximum;
            output.available = true; output.specialMode = active->specialMode;
        }
    }
}
