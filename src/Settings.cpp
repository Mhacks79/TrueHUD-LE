#include "Settings.h"
#include "Runtime.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <algorithm>

namespace TrueHUDLE {
    namespace {
        Settings current;
        SRWLOCK configLock = SRWLOCK_INIT;
        char defaultsPath[MAX_PATH] = {}, overridesPath[MAX_PATH] = {};

        const SettingEntry* Find(const char* id) {
            if (id) for (const auto& e : SettingEntries) if (!strcmp(id, e.id)) return &e;
            return nullptr;
        }
        double Clamp(const SettingEntry& e, double value) {
            if (!std::isfinite(value)) return e.initial;
            value = std::clamp(value, e.minimum, e.maximum);
            return e.integer ? std::floor(value) : value;
        }
        void ReadFile(const char* path, Settings& output) {
            for (const auto& e : SettingEntries) {
                char key[128], text[128];
                strcpy_s(key, e.id);
                char* section = strchr(key, ':');
                if (!section) continue;
                *section++ = 0;
                GetPrivateProfileStringA(section, key, "", text, sizeof(text), path);
                double value;
                if (text[0] && ParseNumber(text, value, e.color))
                    *reinterpret_cast<double*>(reinterpret_cast<char*>(&output) + e.offset) = Clamp(e, value);
            }
        }
    }

    bool ParseNumber(const char* text, double& result, bool hex) {
        if (!text) return false;
        while (*text == ' ' || *text == '\t') ++text;
        bool negative = false;
        if (*text == '-' || *text == '+') negative = *text++ == '-';
        if (hex && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) text += 2;
        double value = 0, divisor = 1;
        bool decimal = false, digit = false;
        for (; *text; ++text) {
            const char c = *text;
            if (c == ' ' || c == '\t' || c == ';' || c == '#') break;
            if (!hex && (c == '.' || c == ',') && !decimal) { decimal = true; continue; }
            unsigned d = c >= '0' && c <= '9' ? c - '0' :
                hex && c >= 'a' && c <= 'f' ? c - 'a' + 10 :
                hex && c >= 'A' && c <= 'F' ? c - 'A' + 10 : 255;
            if (d >= (hex ? 16u : 10u)) return false;
            digit = true;
            if (decimal) { divisor *= 10; value += d / divisor; }
            else value = value * (hex ? 16 : 10) + d;
            if (!std::isfinite(value)) return false;
        }
        if (!digit) return false;
        // Whitespace may precede an INI comment, but not another token.
        while (*text == ' ' || *text == '\t') ++text;
        if (*text && *text != ';' && *text != '#') return false;
        result = negative ? -value : value;
        return true;
    }

    bool InitializeSettings() {
        char executable[MAX_PATH];
        const DWORD count = GetModuleFileNameA(nullptr, executable, MAX_PATH);
        if (!count || count >= MAX_PATH) return false;
        char* leaf = strrchr(executable, '\\');
        if (!leaf) return false;
        *leaf = 0;
        if (sprintf_s(defaultsPath, "%s\\Data\\MCM\\Config\\TrueHUD\\settings.ini", executable) < 0 ||
            sprintf_s(overridesPath, "%s\\Data\\SKSE\\Plugins\\TrueHUD\\TrueHUD_LE.ini", executable) < 0) return false;
        return ReadSettings();
    }
    void CopySettings(Settings& output) {
        AcquireSRWLockShared(&configLock);
        output = current;
        ReleaseSRWLockShared(&configLock);
    }
    double GetSetting(const char* id) {
        const auto e = Find(id);
        if (!e) return 0;
        AcquireSRWLockShared(&configLock);
        const double value = *reinterpret_cast<const double*>(reinterpret_cast<const char*>(&current) + e->offset);
        ReleaseSRWLockShared(&configLock);
        return value;
    }
    bool SetSetting(const char* id, double value) {
        const auto e = Find(id);
        if (!e) return false;
        AcquireSRWLockExclusive(&configLock);
        *reinterpret_cast<double*>(reinterpret_cast<char*>(&current) + e->offset) = Clamp(*e, value);
        ReleaseSRWLockExclusive(&configLock);
        return true;
    }
    bool ReadSettings() {
        Settings next;
        ReadFile(defaultsPath, next);
        ReadFile(overridesPath, next);
        AcquireSRWLockExclusive(&configLock);
        current = next;
        ReleaseSRWLockExclusive(&configLock);
        return true;
    }
    bool WriteSettings() {
        Settings snapshot;
        CopySettings(snapshot);
        bool success = true;
        for (const auto& e : SettingEntries) {
            char key[128], text[64];
            strcpy_s(key, e.id);
            char* section = strchr(key, ':');
            if (!section) continue;
            *section++ = 0;
            const double value = Clamp(e, *reinterpret_cast<const double*>(reinterpret_cast<const char*>(&snapshot) + e.offset));
            if (e.color) sprintf_s(text, "0x%06X", static_cast<unsigned>(value));
            else {
                // Decimal output independent of process C locale, same clamp as read/set.
                const bool negative = value < 0;
                const auto units = static_cast<unsigned long long>(std::floor(std::fabs(value) * 1000000 + 0.5));
                sprintf_s(text, "%s%llu.%06llu", negative ? "-" : "", units / 1000000, units % 1000000);
            }
            if (!WritePrivateProfileStringA(section, key, text, overridesPath)) success = false;
        }
        return success;
    }
}
