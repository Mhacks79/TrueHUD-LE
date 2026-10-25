#include "APICommands.h"
#include "APIUIState.h"
#include "APISafety.h"
#include "SpecialResources.h"
#include "TargetControl.h"
#include "DebugDraw.h"
#include <cstring>

namespace TrueHUDLE {
    namespace {
        using namespace TRUEHUD_API;
        volatile LONG apiFailures = 0;
        template<class F> bool GuardAPI(F&& function) noexcept {
            if (Disabled() || InterlockedCompareExchange(&apiFailures, 0, 0) >= 3) return false;
            if (InvokeClient(std::forward<F>(function))) return true;
            if (InterlockedIncrement(&apiFailures) == 3) Log("TrueHUD public API disabled after three faults");
            return false;
        }
        template<class F> APIResult Result(F&& function) noexcept {
            APIResult result = APIResult::BadThread;
            GuardAPI([&] { result = function(); }); return result;
        }
        template<class F> PluginHandle Owner(F&& function) noexcept {
            PluginHandle result = InvalidResourceOwner;
            GuardAPI([&] { result = function(); }); return result;
        }
        APIResult Translate(ResourceResult result) {
            switch (result) {
            case ResourceResult::OK: return APIResult::OK;
            case ResourceResult::NotOwner: return APIResult::NotOwner;
            case ResourceResult::AlreadyGiven: return APIResult::AlreadyGiven;
            case ResourceResult::AlreadyTaken: return APIResult::AlreadyTaken;
            default: return APIResult::BadThread;
            }
        }
        bool Text(std::string_view text, char (&output)[260], bool path = false) {
            if (text.empty() || text.size() >= sizeof(output) || text.find('\0') != text.npos) return false;
            // Library paths remain relative to Data/Interface, as upstream.
            if (path && (text.front() == '/' || text.front() == '\\' || text.find(':') != text.npos ||
                         text.find("..") != text.npos)) return false;
            memcpy(output, text.data(), text.size()); output[text.size()] = 0; return true;
        }
        bool Queue(APICommand command) {
            if (EnqueueAPICommand(std::move(command))) return true;
            Log("TrueHUD API queue full; request rejected"); return false;
        }
        void ActorCommand(APICommandKind kind, UInt32 actor, UInt32 value = 0, bool longFlash = false, UInt32 owner = 0) {
            if (!actor || actor == 0xFFFFFFFFu) return;
            APICommand command; command.kind = kind; command.actor = actor;
            command.value = value; command.longFlash = longFlash; command.owner = owner;
            Queue(std::move(command));
        }
        class PublicAPI final : public IVTrueHUD4 {
        public:
            unsigned long GetTrueHUDThreadId() const noexcept override { return ReadAPIThread(); }
            APIResult RequestTargetControl(PluginHandle owner) noexcept override {
                return Result([&] { return Translate(RequestTargetControlNative(owner)); });
            }
            APIResult RequestSpecialResourceBarsControl(PluginHandle owner) noexcept override {
                return Result([&] { return Translate(RequestSpecialControl(owner)); });
            }
            APIResult SetTarget(PluginHandle owner, ActorHandle actor) noexcept override {
                return Result([&] { return Translate(SetControlledTarget(owner, actor, false)); });
            }
            APIResult SetSoftTarget(PluginHandle owner, ActorHandle actor) noexcept override {
                return Result([&] { return Translate(SetControlledTarget(owner, actor, true)); });
            }
            void AddActorInfoBar(ActorHandle actor) noexcept override {
                GuardAPI([&] { ActorCommand(APICommandKind::AddActor, actor); });
            }
            void RemoveActorInfoBar(ActorHandle actor, WidgetRemovalMode mode) noexcept override {
                GuardAPI([&] { if (static_cast<unsigned>(mode) <= 2) ActorCommand(APICommandKind::RemoveActor, actor, static_cast<UInt32>(mode)); });
            }
            void AddBoss(ActorHandle actor) noexcept override { GuardAPI([&] { ActorCommand(APICommandKind::AddBoss, actor); }); }
            void RemoveBoss(ActorHandle actor, WidgetRemovalMode mode) noexcept override {
                GuardAPI([&] { if (static_cast<unsigned>(mode) <= 2) ActorCommand(APICommandKind::RemoveBoss, actor, static_cast<UInt32>(mode)); });
            }
            void FlashActorValue(ActorHandle actor, ActorValue value, bool longFlash) noexcept override {
                GuardAPI([&] { if (value >= 24 && value <= 26) ActorCommand(APICommandKind::FlashActor, actor, value, longFlash); });
            }
            APIResult FlashActorSpecialBar(PluginHandle owner, ActorHandle actor, bool longFlash) noexcept override {
                return Result([&] {
                    if (owner == InvalidResourceOwner || ReadSpecialStatus().owner != owner) return APIResult::NotOwner;
                    APICommand command; command.kind = APICommandKind::FlashSpecial; command.owner = owner;
                    command.actor = actor; command.longFlash = longFlash;
                    return Queue(std::move(command)) ? APIResult::OK : APIResult::BadThread;
                });
            }
            APIResult RegisterSpecialResourceFunctions(PluginHandle owner, SpecialResourceCallback&& current,
                SpecialResourceCallback&& maximum, bool mode, bool player) noexcept override {
                return Result([&] { return Translate(RegisterSpecialCallbacks(owner, std::move(current), std::move(maximum), mode, player)); });
            }
            void LoadCustomWidgets(PluginHandle owner, std::string_view path, APIResultCallback&& callback) noexcept override {
                bool queued = false;
                GuardAPI([&] {
                    APICommand command; command.kind = APICommandKind::LoadCustom; command.owner = owner;
                    if (owner == InvalidResourceOwner || !Text(path, command.text, true)) return;
                    // Copy callback so a rejected queue can still report failure.
                    command.callback = callback; queued = Queue(std::move(command));
                });
                // Only rejection runs synchronously. Accepted callbacks run on
                // the UI thread, after loadClip's acceptance result (upstream).
                if (!queued && callback) InvokeClient([&] { callback(APIResult::WidgetFailedToLoad); });
            }
            void RegisterNewWidgetType(PluginHandle owner, uint32_t type) noexcept override {
                GuardAPI([&] { if (owner == InvalidResourceOwner) return;
                    APICommand command; command.kind = APICommandKind::RegisterCustomType;
                    command.owner = owner; command.type = type; Queue(std::move(command)); });
            }
            void AddWidget(PluginHandle owner, uint32_t type, uint32_t id, std::string_view symbol,
                           std::shared_ptr<WidgetBase> widget) noexcept override {
                GuardAPI([&] { if (owner == InvalidResourceOwner || !widget) return;
                    APICommand command; command.kind = APICommandKind::AddCustom;
                    command.owner = owner; command.type = type; command.id = id;
                    if (!Text(symbol, command.text)) return;
                    command.widget = std::move(widget); Queue(std::move(command)); });
            }
            void RemoveWidget(PluginHandle owner, uint32_t type, uint32_t id, WidgetRemovalMode mode) noexcept override {
                GuardAPI([&] { if (owner == InvalidResourceOwner || static_cast<unsigned>(mode) > 2) return;
                    APICommand command; command.kind = APICommandKind::RemoveCustom;
                    command.owner = owner; command.type = type; command.id = id;
                    command.value = static_cast<UInt32>(mode); Queue(std::move(command)); });
            }
            PluginHandle GetTargetControlOwner() const noexcept override { return Owner([] { return ReadTargetControlOwner(); }); }
            PluginHandle GetPlayerWidgetBarColorsControlOwner() const noexcept override { return InvalidResourceOwner; }
            PluginHandle GetSpecialResourceBarControlOwner() const noexcept override { return Owner([] { return ReadSpecialStatus().owner; }); }
            APIResult ReleaseTargetControl(PluginHandle owner) noexcept override {
                return Result([&] { return Translate(ReleaseTargetControlNative(owner)); });
            }
            APIResult ReleaseSpecialResourceBarControl(PluginHandle owner) noexcept override {
                return Result([&] { return Translate(ReleaseSpecialControl(owner)); });
            }
            void Color(ActorHandle actor, unsigned bar, BarColorType type, UInt32 color, bool revert) noexcept {
                GuardAPI([&] {
                    if (!actor || actor == 0xFFFFFFFFu || bar >= 4 || static_cast<unsigned>(type) >= 5) return;
                    APICommand command; command.kind = revert ? APICommandKind::RevertColor : APICommandKind::OverrideColor;
                    command.actor = actor; command.type = bar; command.id = static_cast<UInt32>(type); command.value = color;
                    Queue(std::move(command));
                });
            }
            void OverrideBarColor(ActorHandle actor, ActorValue av, BarColorType type, uint32_t color) noexcept override {
                if (av >= 24 && av <= 26) Color(actor, av - 24, type, color, false);
            }
            void OverrideSpecialBarColor(ActorHandle actor, BarColorType type, uint32_t color) noexcept override { Color(actor, 3, type, color, false); }
            void RevertBarColor(ActorHandle actor, ActorValue av, BarColorType type) noexcept override {
                if (av >= 24 && av <= 26) Color(actor, av - 24, type, 0, true);
            }
            void RevertSpecialBarColor(ActorHandle actor, BarColorType type) noexcept override { Color(actor, 3, type, 0, true); }
            void DrawLine(const NiPoint3& a, const NiPoint3& b, float duration, uint32_t color, float thickness) noexcept override {
                GuardAPI([&] { DebugLine3D(a,b,duration,color,thickness); });
            }
            void DrawPoint(const NiPoint3& point, float size, float duration, uint32_t color) noexcept override {
                GuardAPI([&] { DebugLine3D(point,point,duration,color,1,size); });
            }
            void DrawArrow(const NiPoint3& a, const NiPoint3& b, float size, float duration, uint32_t color, float thickness) noexcept override {
                GuardAPI([&] { DebugArrow(a,b,size,duration,color,thickness); });
            }
            void DrawBox(const NiPoint3& center, const NiPoint3& extent, const NiQuaternion& rotation,
                         float duration, uint32_t color, float thickness) noexcept override {
                GuardAPI([&] { DebugBox(center,extent,rotation,duration,color,thickness); });
            }
            void DrawCircle(const NiPoint3& center, const NiPoint3& x, const NiPoint3& y, float radius, uint32_t segments,
                            float duration, uint32_t color, float thickness) noexcept override {
                GuardAPI([&] { DebugCircle(center,x,y,radius,segments,duration,color,thickness); });
            }
            void DrawHalfCircle(const NiPoint3& center, const NiPoint3& x, const NiPoint3& y, float radius, uint32_t segments,
                                float duration, uint32_t color, float thickness) noexcept override {
                GuardAPI([&] { DebugCircle(center,x,y,radius,segments,duration,color,thickness,true); });
            }
            void DrawSphere(const NiPoint3& center, float radius, uint32_t segments, float duration, uint32_t color, float thickness) noexcept override {
                GuardAPI([&] { DebugSphere(center,radius,segments,duration,color,thickness); });
            }
            void DrawCylinder(const NiPoint3& a, const NiPoint3& b, float radius, uint32_t segments,
                              float duration, uint32_t color, float thickness) noexcept override {
                GuardAPI([&] { DebugCylinder(a,b,radius,segments,duration,color,thickness); });
            }
            void DrawCone(const NiPoint3& origin, const NiPoint3& direction, float length, float width, float height,
                          uint32_t segments, float duration, uint32_t color, float thickness) noexcept override {
                GuardAPI([&] { DebugCone(origin,direction,length,width,height,segments,duration,color,thickness); });
            }
            void DrawCapsule(const NiPoint3& origin, float halfHeight, float radius, const NiQuaternion& rotation,
                             float duration, uint32_t color, float thickness) noexcept override {
                GuardAPI([&] { DebugCapsule(origin,halfHeight,radius,rotation,duration,color,thickness); });
            }
            bool HasInfoBar(ActorHandle actor, bool floatingOnly) const noexcept override {
                return HasPublishedInfoBar(actor,floatingOnly);
            }
            void DrawCapsule(const NiPoint3& a, const NiPoint3& b, float radius, float duration, uint32_t color, float thickness) noexcept override {
                GuardAPI([&] { DebugCapsule(a,b,radius,duration,color,thickness); });
            }
        } service;
    }
    void* RequestAPI(TRUEHUD_API::InterfaceVersion version) noexcept {
        switch (version) {
        case TRUEHUD_API::InterfaceVersion::V1: return static_cast<TRUEHUD_API::IVTrueHUD1*>(&service);
        case TRUEHUD_API::InterfaceVersion::V2: return static_cast<TRUEHUD_API::IVTrueHUD2*>(&service);
        case TRUEHUD_API::InterfaceVersion::V3: return static_cast<TRUEHUD_API::IVTrueHUD3*>(&service);
        case TRUEHUD_API::InterfaceVersion::V4: return static_cast<TRUEHUD_API::IVTrueHUD4*>(&service);
        default: return nullptr; // No unimplemented virtual methods advertised.
        }
    }
}
extern "C" void* __cdecl RequestPluginAPI(TRUEHUD_API::InterfaceVersion version) {
    return TrueHUDLE::RequestAPI(version);
}
