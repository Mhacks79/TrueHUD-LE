#include "Runtime.h"
#include "Signatures.h"
#include <cstring>
namespace TrueHUDLE {
    bool VerifyEngine() {
        unsigned mismatches = 0, hooks = 0;
        for (const auto& site : signatures) {
            const auto bytes = reinterpret_cast<const unsigned char*>(site.address);
            if (!Readable(bytes, sizeof(site.bytes), false)) { ++mismatches; continue; }
            if (!memcmp(bytes, site.bytes, sizeof(site.bytes))) continue;
            unsigned first = 0;
            while (first < sizeof(site.bytes) && bytes[first] == site.bytes[first]) ++first;
            // First changed byte may be inside a prologue (allocator hooks, F7).
            const auto p = bytes + first;
            const bool jump = first + 6 <= sizeof(site.bytes) &&
                (p[0] == 0xE9 || p[0] == 0xE8 || p[0] == 0xEB ||
                 (p[0] == 0xFF && p[1] == 0x25) || (p[0] == 0x68 && p[5] == 0xC3));
            if (jump) { ++hooks; Log("existing hook at %s+%u", site.name, first); }
            else { ++mismatches; Log("unexplained signature difference at %s+%u", site.name, first); }
        }
        Log("startup signatures: sites=%u existing-hooks=%u differences=%u", unsigned(_countof(signatures)), hooks, mismatches);
        return mismatches * 4 <= _countof(signatures) * 3;
    }
}
