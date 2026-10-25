// A movie address is an identity token, never dereferenced after replacement.
#pragma once
#include <cmath>
namespace TrueHUDLE {
    template<class Access> class HUDNumberOverride {
        const char* path;
        typename Access::Movie* owner = nullptr;
        double original = 0;
    public:
        explicit HUDNumberOverride(const char* name) : path(name) {}
        void Apply(typename Access::Movie* current, bool enabled, double value, bool relative = false, bool continuous = true) {
            if (owner != current) owner = nullptr;
            if (!enabled) { Restore(current); return; }
            if (!current || !std::isfinite(value)) return;
            if (owner && !continuous) return;
            if (!owner) {
                if (!Access::Read(current, path, original) || !std::isfinite(original)) return;
                owner = current;
            }
            Access::Write(current, path, relative ? original + value : value);
        }
        void Restore(typename Access::Movie* current) {
            if (owner && owner == current) Access::Write(current, path, original);
            owner = nullptr;
        }
    };
}
