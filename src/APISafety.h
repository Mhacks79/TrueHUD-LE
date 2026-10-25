#pragma once
#include "Runtime.h"

namespace TrueHUDLE {
    // Keep SEH outside C++ frames that own captures/locks. Neither callback
    // execution nor capture destruction is performed with a registry lock held.
    template<class F> bool ClientCpp(F& function) {
        try { function(); return true; }
        catch (...) { Log("TrueHUD API client C++ exception, thread=%lu", GetCurrentThreadId()); return false; }
    }
    template<class F> bool InvokeClient(F&& function) {
        __try { return ClientCpp(function); }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            Log("TrueHUD API client SEH exception=%08lX, thread=%lu", GetExceptionCode(), GetCurrentThreadId());
            return false;
        }
    }
}
