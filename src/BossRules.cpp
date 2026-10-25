#include "Runtime.h"
#include "Settings.h"
#include "skse/GameData.h"
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

namespace TrueHUDLE {
    namespace {
        std::vector<UInt32> races, npcs, blacklist, locations;
        UInt32 locationBossRefs[1024] = {};
        unsigned locationBossCount = 0;
        UInt32 tracedBossRefs[24]{};
        unsigned tracedBossCount = 0;
        UInt32 Resolve(const std::string& value) {
            const auto colon = value.find(':');
            if (colon == std::string::npos) return 0;
            const std::string plugin = value.substr(0, colon);
            double local;
            if (!ParseNumber(value.c_str() + colon + 1, local, true) || local < 0 || local > 0xFFFFFF) return 0;
            auto data = *reinterpret_cast<DataHandler**>(0x012E2CB8);
            if (!Readable(data, sizeof(DataHandler)) || data->modList.loadedModCount > 255) return 0;
            for (UInt32 i = 0; i < data->modList.loadedModCount; ++i) {
                const ModInfo* mod = data->modList.loadedMods[i];
                if (Readable(mod, sizeof(ModInfo)) && !_stricmp(plugin.c_str(), mod->name) && mod->modIndex != 0xFF)
                    return static_cast<UInt32>(local) | (UInt32(mod->modIndex) << 24);
            }
            return 0;
        }
        void Read(const char* path) {
            std::ifstream stream(path);
            std::string line, section;
            while (std::getline(stream, line)) {
                const auto comment = line.find_first_of(";#");
                if (comment != std::string::npos) line.resize(comment);
                const auto last = line.find_last_not_of(" \t\r\n");
                if (last == std::string::npos) continue;
                line.resize(last + 1);
                line.erase(0, line.find_first_not_of(" \t"));
                if (line.front() == '[') { section = line; continue; }
                if (_stricmp(section.c_str(), "[BossRecognition]")) continue;
                const auto equal = line.find('=');
                if (equal == std::string::npos) continue;
                std::string key = line.substr(0, equal);
                key.resize(key.find_last_not_of(" \t") + 1);
                std::string value = line.substr(equal + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                const bool remove = key.compare(0, 6, "Remove") == 0;
                if (remove) key.erase(0, 6);
                auto list = key == "Race" ? &races : key == "NPC" ? &npcs : key == "NPCBlacklist" ? &blacklist : key == "LocRefType" ? &locations : nullptr;
                if (!list) continue;
                const UInt32 id = Resolve(value);
                if (!id) continue;
                const auto found = std::find(list->begin(), list->end(), id);
                if (remove && found != list->end()) list->erase(found);
                else if (!remove && found == list->end()) list->push_back(id);
            }
        }
        void Directory(const char* root, const char* folder, const char* base) {
            char pattern[MAX_PATH], path[MAX_PATH];
            sprintf_s(path, "%s\\Data\\SKSE\\Plugins\\%s\\%s", root, folder, base);
            Read(path);
            sprintf_s(pattern, "%s\\Data\\SKSE\\Plugins\\%s\\*.ini", root, folder);
            WIN32_FIND_DATAA entry;
            HANDLE search = FindFirstFileA(pattern, &entry);
            if (search == INVALID_HANDLE_VALUE) return;
            std::vector<std::string> names;
            do {
                if (!(entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && _stricmp(entry.cFileName, base))
                    names.emplace_back(entry.cFileName);
            } while (FindNextFileA(search, &entry));
            FindClose(search);
            std::sort(names.begin(), names.end());
            for (const auto& name : names) {
                sprintf_s(path, "%s\\Data\\SKSE\\Plugins\\%s\\%s", root, folder, name.c_str());
                Read(path);
            }
        }
    }
    void InitializeBossRules() {
        tracedBossCount = 0;
        races.clear(); npcs.clear(); blacklist.clear(); locations.clear();
        char root[MAX_PATH];
        const DWORD length = GetModuleFileNameA(nullptr, root, MAX_PATH);
        if (!length || length >= MAX_PATH) return;
        char* leaf = strrchr(root, '\\');
        if (!leaf) return;
        *leaf = 0;
        Directory(root, "TrueDirectionalMovement", "TrueDirectionalMovement_base.ini");
        Directory(root, "TrueHUD", "TrueHUD_base.ini");
        Log("boss rules: races=%u NPCs=%u blacklist=%u location-types=%u", unsigned(races.size()), unsigned(npcs.size()), unsigned(blacklist.size()), unsigned(locations.size()));
    }
    void PrepareLocationBosses(Actor* player) {
        locationBossCount = 0;
        if (!Readable(player, sizeof(PlayerCharacter))) return;
        auto location = *reinterpret_cast<BGSLocation**>(reinterpret_cast<char*>(player) + 0x6B0);
        if (!Readable(location, sizeof(BGSLocation)) || location->formType != kFormType_Location) return;
        // Ghidra 004B1510: pointer at40, count48, stride16. Init004AFF00
        // resolves each entry+4 through LookupFormByID; entry0 is ref-type form.
        struct SpecialReference { TESForm* type; UInt32 refID, reserved[2]; };
        static_assert(sizeof(SpecialReference) == 16, "LE location special reference stride");
        auto refs = *reinterpret_cast<SpecialReference**>(reinterpret_cast<char*>(location) + 0x40);
        const UInt32 count = *reinterpret_cast<UInt32*>(reinterpret_cast<char*>(location) + 0x48);
        if (!count || count > 65536 || !Readable(refs, size_t(count) * sizeof(SpecialReference))) return;
        for (UInt32 i = 0; i < count && locationBossCount < _countof(locationBossRefs); ++i) {
            if (Readable(refs[i].type, sizeof(TESForm)) &&
                std::find(locations.begin(), locations.end(), refs[i].type->formID) != locations.end())
                locationBossRefs[locationBossCount++] = refs[i].refID;
        }
    }
    UInt32 OriginalBaseID(const Actor* actor) {
        if (!Readable(actor, sizeof(Actor))) return 0;
        const auto& extra = actor->extraData;
        if (!Readable(extra.m_presence, sizeof(BaseExtraList::PresenceBitfield)) ||
            !extra.m_presence->HasType(0x2D)) return 0;
        auto item = extra.m_data;
        // Optional read-only lookup on the game task while Actor is pinned.
        // Avoid native GetExtraData's TLS cache/global lock: a bad optional
        // chain must fail closed without entering an engine lock.
        for (unsigned budget = 0; item && budget < 1024; ++budget) {
            if (!Readable(item, sizeof(BSExtraData))) return 0;
            if (*reinterpret_cast<const UInt32*>(item) == 0x01079D1C) {
                if (!Readable(item, 16)) return 0;
                const auto base = *reinterpret_cast<TESForm* const*>(reinterpret_cast<const char*>(item)+8);
                return Readable(base, sizeof(TESForm)) && base->formType == kFormType_NPC ? base->formID : 0;
            }
            item = item->next;
        }
        return 0;
    }
    bool IsBoss(Actor* actor) {
        if (!actor || !Readable(actor->baseForm, sizeof(TESForm))) return false;
        const UInt32 base = actor->baseForm->formID;
        const UInt32 original = OriginalBaseID(actor);
        auto listed = [&](const std::vector<UInt32>& list) {
            return std::find(list.begin(), list.end(), base) != list.end() ||
                (original && std::find(list.begin(), list.end(), original) != list.end());
        };
        const bool excluded = listed(blacklist), npc = listed(npcs);
        const bool location = std::find(locationBossRefs, locationBossRefs + locationBossCount, actor->formID) != locationBossRefs + locationBossCount;
        const UInt32 race = Readable(actor->race, sizeof(TESForm)) ? actor->race->formID : 0;
        const bool raceMatch = race && std::find(races.begin(), races.end(), race) != races.end();
        const bool recognized = !excluded && (npc || location || raceMatch);
        if (tracedBossCount < _countof(tracedBossRefs) &&
            std::find(tracedBossRefs, tracedBossRefs + tracedBossCount, actor->formID) == tracedBossRefs + tracedBossCount) {
            tracedBossRefs[tracedBossCount++] = actor->formID;
            Log("boss classification: ref=%08X base=%08X original=%08X race=%08X blacklist=%u NPC=%u location=%u race-match=%u recognized=%u",
                actor->formID, base, original, race, excluded, npc, location, raceMatch, recognized);
        }
        return recognized;
    }
}
