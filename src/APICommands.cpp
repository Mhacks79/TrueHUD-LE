#include "APICommands.h"
#include <mutex>
#include <utility>

namespace TrueHUDLE {
    namespace {
        std::mutex queueMutex;
        APICommand pending[MaxAPICommands];
        unsigned head = 0, count = 0;
        UInt32 generation = 1;
        bool Persistent(APICommandKind kind) {
            return kind == APICommandKind::LoadCustom || kind == APICommandKind::RegisterCustomType;
        }
    }
    bool EnqueueAPICommand(APICommand command) {
        std::lock_guard<std::mutex> guard(queueMutex);
        if (count == MaxAPICommands) return false;
        command.generation = generation;
        std::swap(command, pending[(head + count) % MaxAPICommands]);
        ++count;
        return true;
    }
    bool PopAPICommand(APICommand& output, bool& currentGeneration) {
        output = {}; // Client capture/widget destructors can re-enter this queue.
        std::lock_guard<std::mutex> guard(queueMutex);
        if (!count) { currentGeneration = false; return false; }
        std::swap(output, pending[head]);
        head = (head + 1) % MaxAPICommands; --count;
        currentGeneration = Persistent(output.kind) || output.generation == generation;
        return true;
    }
    void ResetAPICommands() {
        std::lock_guard<std::mutex> guard(queueMutex);
        if (!++generation) ++generation;
        // Keep library/type definitions. Transient jobs are consumed/disposed
        // on the UI thread, never destroyed by a save/load worker callback.
    }
}
