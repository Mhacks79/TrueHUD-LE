#pragma once
#include "Runtime.h"
#include "Settings.h"
namespace TrueHUDLE {
    // Owned by one serialized sampler or menu. Diagnostic wall time includes
    // engine work/waits; it is not a GPU timer or an FPS measurement.
    class Timing {
        struct Phase { unsigned count = 0; ULONGLONG sum = 0, peak = 0; } phases[3];
        const char* tag;
        ULONGLONG frequency = 0, nextReport = 0;
        unsigned reports = 0, maxBosses = 0;
        ULONGLONG Micros(ULONGLONG value) const { return frequency ? value * 1000000 / frequency : 0; }
    public:
        explicit Timing(const char* name) : tag(name) {}
        ULONGLONG Begin() const {
            if (!LoggingEnabled() || reports >= 12) return 0;
            LARGE_INTEGER time{}; QueryPerformanceCounter(&time); return time.QuadPart;
        }
        void End(unsigned phase, ULONGLONG start, unsigned bosses, bool report = false) {
            if (!start || phase >= 3) return;
            LARGE_INTEGER time{}; QueryPerformanceCounter(&time);
            if (time.QuadPart <= 0) return;
            const ULONGLONG current = static_cast<ULONGLONG>(time.QuadPart);
            if (!frequency) { LARGE_INTEGER value{}; QueryPerformanceFrequency(&value); if (value.QuadPart > 0) frequency = static_cast<ULONGLONG>(value.QuadPart); }
            if (!frequency || current < start) return;
            const ULONGLONG elapsed = current-start;
            auto& p = phases[phase]; ++p.count; p.sum += elapsed; if (elapsed > p.peak) p.peak = elapsed;
            if (bosses > maxBosses) maxBosses = bosses;
            if (!nextReport) nextReport = current + 5*frequency;
            if (!report || current < nextReport) return;
            Log("perf %s bosses=%u peak-bosses=%u p0-count=%u p0-us=%llu p0-peak-us=%llu p1-count=%u p1-us=%llu p1-peak-us=%llu p2-count=%u p2-us=%llu p2-peak-us=%llu",
                tag, bosses, maxBosses, phases[0].count, Micros(phases[0].sum), Micros(phases[0].peak),
                phases[1].count, Micros(phases[1].sum), Micros(phases[1].peak),
                phases[2].count, Micros(phases[2].sum), Micros(phases[2].peak));
            for (auto& item : phases) item = {};
            maxBosses = 0; nextReport = current + 5*frequency; ++reports;
        }
    };
}
