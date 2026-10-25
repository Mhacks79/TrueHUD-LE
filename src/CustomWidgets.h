#pragma once
#include "APICommands.h"
#include "APISafety.h"
#include <cstring>
#include <utility>

namespace TrueHUDLE {
    struct CustomDefinitions {
        struct Library { UInt32 owner = 0; bool used = false; char path[260] = {}; } libraries[32];
        struct Type { UInt32 owner = 0, type = 0; bool used = false; } types[128];
    };
    // Host provides GFx operations on the menu thread. The same lifecycle is
    // exercised by the native harness without pretending to execute Scaleform.
    template<class Host> class CustomWidgetRegistry {
        using Base = TRUEHUD_API::WidgetBase;
        struct Slot {
            UInt32 owner = 0, type = 0, id = 0;
            std::shared_ptr<Base> widget;
            char symbol[260] = {};
            float wait = 0;
            unsigned failures = 0;
            bool attached = false;
        } slots[64];
        Host& host;
        CustomDefinitions& definitions;
        bool loaded[32] = {};
        bool restored = false;
        void Drop(Slot& slot) {
            // Remove it from the registry first, so client re-entry cannot
            // observe a half-disposed widget. All calls are on the UI thread.
            Slot old = std::move(slot); slot = {};
            if (!old.widget) return;
            if (old.attached) {
                InvokeClient([&] { old.widget->Dispose(); });
                host.Detach(old.owner, old.type, old.id);
            }
            old.widget->_widgetState = Base::kRemoved;
            old.widget->_object.SetUndefined();
            old.widget->_view.reset();
        }
        bool TypeKnown(UInt32 owner, UInt32 type) const {
            for (const auto& item : definitions.types)
                if (item.used && item.owner == owner && item.type == type) return true;
            return false;
        }
    public:
        CustomWidgetRegistry(Host& h, CustomDefinitions& d) : host(h), definitions(d) {}
        void RestoreDefinitions() {
            if (restored) return;
            for (unsigned i = 0; i < 32; ++i) {
                const auto& lib = definitions.libraries[i];
                if (lib.used && (loaded[i] = host.Load(lib.owner, lib.path)))
                    for (const auto& type : definitions.types) if (type.used && type.owner == lib.owner)
                        host.Register(type.owner, type.type);
            }
            restored = true;
        }
        void Clear() { for (auto& slot : slots) Drop(slot); }
        bool Process(APICommand& command) {
            RestoreDefinitions();
            switch (command.kind) {
            case APICommandKind::LoadCustom: {
                CustomDefinitions::Library* entry = nullptr;
                for (auto& lib : definitions.libraries) {
                    if (lib.used && lib.owner == command.owner) { entry = &lib; break; }
                    if (!lib.used && !entry) entry = &lib;
                }
                bool success = entry != nullptr;
                const bool same = success && entry->used && loaded[entry-definitions.libraries] && !strcmp(entry->path, command.text);
                if (success && !same) {
                    for (auto& slot : slots) if (slot.widget && slot.owner == command.owner) Drop(slot);
                    success = host.Load(command.owner, command.text);
                    loaded[entry-definitions.libraries] = success;
                    if (success) {
                        entry->used = true; entry->owner = command.owner;
                        memcpy(entry->path, command.text, sizeof(entry->path));
                        for (const auto& type : definitions.types) if (type.used && type.owner == command.owner)
                            host.Register(type.owner, type.type);
                    } else if (entry->used) entry->used = false;
                }
                if (command.callback) InvokeClient([&] {
                    command.callback(success ? TRUEHUD_API::APIResult::OK : TRUEHUD_API::APIResult::WidgetFailedToLoad);
                });
                return true;
            }
            case APICommandKind::RegisterCustomType:
                if (!TypeKnown(command.owner, command.type)) {
                    bool library = false;
                    for (const auto& lib : definitions.libraries) if (lib.used && lib.owner == command.owner) library = true;
                    if (!library) return true;
                    for (auto& type : definitions.types) if (!type.used) {
                        type = {command.owner, command.type, true};
                        host.Register(command.owner, command.type); break;
                    }
                }
                return true;
            case APICommandKind::AddCustom: {
                if (!command.widget || !TypeKnown(command.owner, command.type)) return true;
                // One client object cannot back two clips or already own a view.
                if (command.widget->_view || command.widget->_object.IsDisplayObject()) return true;
                for (auto& slot : slots) if (slot.widget == command.widget) return true;
                for (auto& slot : slots) if (slot.widget && slot.owner == command.owner &&
                    slot.type == command.type && slot.id == command.id) Drop(slot);
                for (auto& slot : slots) if (!slot.widget) {
                    slot.owner = command.owner; slot.type = command.type; slot.id = command.id;
                    memcpy(slot.symbol, command.text, sizeof(slot.symbol));
                    slot.widget = std::move(command.widget); slot.widget->_widgetID = slot.id;
                    break;
                }
                return true;
            }
            case APICommandKind::RemoveCustom:
                for (auto& slot : slots) if (slot.widget && slot.owner == command.owner &&
                    slot.type == command.type && slot.id == command.id) {
                    if (!command.value || !slot.attached) Drop(slot);
                    else if (!InvokeClient([&] { slot.widget->SetWidgetState(command.value == 2 ?
                        Base::kDelayedRemoval : Base::kPendingRemoval); })) Drop(slot);
                    break;
                }
                return true;
            default: return false;
            }
        }
        void Tick(float delta) {
            for (auto& slot : slots) {
                if (!slot.widget) continue;
                if (!slot.attached) {
                    if (!host.Attach(slot.owner, slot.type, slot.id, slot.symbol, *slot.widget)) {
                        slot.wait += delta;
                        if (slot.wait >= 5) Drop(slot);
                        continue;
                    }
                    slot.attached = true;
                    slot.widget->_widgetState = Base::kActive;
                    if (!InvokeClient([&] { slot.widget->Initialize(); })) { Drop(slot); continue; }
                }
                const bool success = InvokeClient([&] { slot.widget->ProcessDelegates(); slot.widget->Update(delta); });
                if (!success && ++slot.failures >= 3) { Drop(slot); continue; }
                if (slot.widget->_widgetState == Base::kRemoved) Drop(slot);
            }
        }
        template<class Visitor> void Visit(Visitor visitor) {
            for (auto& slot : slots) if (slot.widget && slot.attached)
                visitor(slot.owner, slot.type, slot.id, *slot.widget);
        }
    };
}
