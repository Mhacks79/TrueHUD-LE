#pragma once
#include <algorithm>
#include <cmath>
namespace TrueHUDLE {
    inline float ShoutProgress(float recovery, float& cooldown, float finishTime) {
        if (!std::isfinite(recovery) || recovery <= 1e-4f) { cooldown = 0; return 1; }
        cooldown = std::max(cooldown, recovery);
        if (!std::isfinite(finishTime) || finishTime <= 0)
            return std::clamp(1.f - recovery / cooldown, 0.f, 1.f);
        // Vanilla LE0085F140: first80%, then last20% during its INI finish time.
        const float percent = recovery <= finishTime ?
            .8f + (finishTime - recovery) / finishTime * .2f :
            (recovery - cooldown) / (finishTime - cooldown) * .8f;
        return std::clamp(percent, 0.f, 1.f);
    }
    inline double DistanceScale(float cameraDistance, bool enabled, double configuredScale) {
        const float distance = std::isfinite(cameraDistance) ? cameraDistance : 2000.f;
        const double percent = enabled ? 100. - (std::clamp(distance, 500.f, 2000.f) - 500.) / 30. : 100.;
        return percent * configuredScale;
    }
}
