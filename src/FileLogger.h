#pragma once
#include <windows.h>
#include <cstdio>
#include <cstdarg>
namespace TrueHUDLE {
    // One process-lifetime stream; disabled writes do not lock, open or format.
    class FileLogger {
        FILE* stream = nullptr;
        SRWLOCK lock = SRWLOCK_INIT;
        char path[MAX_PATH] = {};
    public:
        explicit FileLogger(FILE* existing = nullptr) : stream(existing) {}
        void SetPath(const char* value) { strcpy_s(path, value); } // load thread, before registration
        void Write(bool enabled, const char* format, va_list args) {
            if (!enabled) return;
            AcquireSRWLockExclusive(&lock);
            if (!stream && path[0]) fopen_s(&stream, path, "w");
            if (stream) { vfprintf(stream, format, args); fputc('\n', stream); fflush(stream); }
            ReleaseSRWLockExclusive(&lock);
        }
    };
}
