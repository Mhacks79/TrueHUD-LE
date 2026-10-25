#pragma once
#include <new>
#include <type_traits>

namespace TrueHUDLE {
    // Engine dispatchers retain these addresses until process termination.
    // Construct once on the serialized registration path. No CRT destructor
    // may turn their vtables into abstract base tables before the engine stops.
    template<class T> class ProcessLifetime final {
        alignas(T) unsigned char storage[sizeof(T)]{};
        T* value = nullptr;
    public:
        ProcessLifetime() = default;
        ProcessLifetime(const ProcessLifetime&) = delete;
        ProcessLifetime& operator=(const ProcessLifetime&) = delete;
        T& Get() {
            if (!value) value = new (storage) T;
            return *value;
        }
    };
}
