#include "Runtime.h"
#include "Settings.h"
#include "HUDOverride.h"
#include "WidgetPolicy.h"
#include "HUDSignals.h"
#include "CustomWidgets.h"
#include "APIUIState.h"
#include "SpecialResources.h"
#include "DebugDraw.h"
#include "DisplayMemo.h"
#include "Timing.h"
#include "LootNotifications.h"
#include "skse/Hooks_UI.h"
#include "skse/ScaleformAPI.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace TrueHUDLE {
    namespace {
        BSFixedString menuName, hudName;
        SKSETaskInterface* uiTasks = nullptr;
        volatile LONG openPending = 0;
        bool registered = false;
        DWORD uiThread = 0;
        CustomDefinitions customDefinitions; // UI-owned, survives menu recreation.
        DebugLine debugScratch[MaxDebugLines]; // UI thread only, never a worker stack array.

        #include "WidgetContracts.inl"

        struct Widget {
            UInt32 handle = 0;
            bool boss = false, configured = false;
            GFxValue object;
            GFxValue depthRow;
            float lastHealth = 0;
            float depth = 0;
            bool specialAvailable = false;
            UInt32 colorRevision = 0;
            ColorRevisionMemo colorMemo;
            UInt8 removal = 255;
            DisplayMemo position;
            double sortedDepth = 0;
        };
        struct FloatingWidget {
            UInt32 serial = 0;
            NiPoint3 position;
            float age = 0, velocity = 0, depth = 0;
            UInt32 color = 0;
            char text[32] = {};
            GFxValue object, depthRow;
            double sortedDepth = 0;
        };
        struct CustomHost {
            GFxMovieView*& view;
            struct Row { UInt32 owner = 0, type = 0, id = 0; bool used = false, known = false; double lastDepth = 0; GFxValue value; } rows[64];
            struct Depth { UInt32 owner = 0; bool used = false, changed = true; GFxValue array; } depths[32];
            bool dirty = true;
            bool Load(UInt32 owner, const char* path) {
                GFxValue args[2], result; args[0].SetNumber(owner); args[1].SetString(path);
                return view->Invoke("_root.TrueHUD.LoadCustomWidgets", &result, args, 2) &&
                    result.GetType() == GFxValue::kType_Bool && result.GetBool();
            }
            void Register(UInt32 owner, UInt32 type) {
                GFxValue args[2]; args[0].SetNumber(owner); args[1].SetNumber(type);
                view->Invoke("_root.TrueHUD.RegisterNewWidgetType", nullptr, args, 2);
            }
            bool Attach(UInt32 owner, UInt32 type, UInt32 id, const char* symbol, TRUEHUD_API::WidgetBase& widget) {
                GFxValue args[4]; args[0].SetNumber(owner); args[1].SetNumber(type);
                args[2].SetNumber(id); args[3].SetString(symbol);
                widget._object.SetUndefined();
                view->Invoke("_root.TrueHUD.AddCustomWidget", &widget._object, args, 4);
                if (!widget._object.IsDisplayObject()) return false;
                widget._view = TRUEHUD_API::RetainMovie(view);
                for (auto& row : rows) if (!row.used) {
                    row.owner = owner; row.type = type; row.id = id; row.used = true; dirty = true; break;
                }
                return true;
            }
            void Detach(UInt32 owner, UInt32 type, UInt32 id) {
                GFxValue args[3]; args[0].SetNumber(owner); args[1].SetNumber(type); args[2].SetNumber(id);
                view->Invoke("_root.TrueHUD.RemoveCustomWidget", nullptr, args, 3);
                for (auto& row : rows) if (row.used && row.owner == owner && row.type == type && row.id == id) {
                    row.used = false; row.value.SetUndefined(); dirty = true;
                }
            }
            void ClearDepths() {
                for (auto& row : rows) { row.used = false; row.value.SetUndefined(); }
                for (auto& depth : depths) { depth.used = false; depth.array.SetUndefined(); }
                dirty = true;
            }
            template<class Registry> void Sort(Registry& registry) {
                if (dirty) {
                    for (auto& depth : depths) { depth.used = false; depth.array.SetUndefined(); }
                    for (auto& row : rows) if (row.used) {
                        Depth* selected = nullptr;
                        for (auto& depth : depths) {
                            if (depth.used && depth.owner == row.owner) { selected = &depth; break; }
                            if (!depth.used && !selected) selected = &depth;
                        }
                        if (!selected) return;
                        if (!selected->used) {
                            selected->owner = row.owner; view->CreateArray(&selected->array);
                            if (selected->array.GetType() != GFxValue::kType_Array) return;
                            selected->used = true; selected->changed = true;
                        }
                        row.value.SetUndefined(); view->CreateObject(&row.value);
                        if (!row.value.IsObject()) return;
                        GFxValue value;
                        value.SetNumber(row.id); if (!row.value.SetMember("id", &value)) return;
                        value.SetNumber(row.type); if (!row.value.SetMember("widgetType", &value)) return;
                        value.SetNumber(0); if (!row.value.SetMember("zIndex", &value)) return;
                        if (!selected->array.PushBack(&row.value)) return;
                        row.known = false;
                    }
                    dirty = false;
                }
                registry.Visit([&](UInt32 owner, UInt32 type, UInt32 id, TRUEHUD_API::WidgetBase& widget) {
                    for (auto& row : rows) if (row.used && row.owner == owner && row.type == type && row.id == id) {
                        const double z = std::isfinite(widget._depth) ? widget._depth : 0;
                        if (!row.known || row.lastDepth != z) {
                            GFxValue value; value.SetNumber(z);
                            if (!row.value.SetMember("zIndex", &value)) return;
                            row.lastDepth = z; row.known = true;
                            for (auto& depth : depths) if (depth.used && depth.owner == owner) depth.changed = true;
                        }
                        break;
                    }
                });
                for (auto& depth : depths) if (depth.used && depth.changed) {
                    // GFxValue has no safe shallow copy in the LE SDK. Put the
                    // array directly in its final argument slot through GFx.
                    GFxValue args[2]; args[0].SetNumber(depth.owner);
                    view->SetVariable("_root.__TrueHUDCustomDepth", &depth.array, 0);
                    if (view->GetVariable(&args[1], "_root.__TrueHUDCustomDepth") &&
                        view->Invoke("_root.TrueHUD.SortCustomWidgetDepths", nullptr, args, 2)) depth.changed = false;
                }
            }
        };
        float Percent(float current, float maximum) {
            // TrueHUD_Bar.updatePercent expects a unit fraction; AS2 applies *100
            // to the mask scale. Opacity and native charge meters use other units.
            return maximum > 0 ? std::clamp(current / maximum, 0.f, 1.f) : 0.f;
        }
        void Position(GFxValue& object, double x, double y, double scale, bool visible = true, DisplayMemo* memo = nullptr) {
            if (memo && memo->Matches(object.data.obj, x, y, scale, visible)) return;
            GFxValue::DisplayInfo display;
            display.SetPosition(x, y);
            display.SetScale(scale, scale);
            display.SetVisible(visible);
            if (object.SetDisplayInfo(&display) && memo) memo->Remember(object.data.obj, x, y, scale, visible);
        }
        struct HUDNumberAccess {
            using Movie = GFxMovieView;
            static bool Read(Movie* movie, const char* path, double& result) {
                GFxValue value;
                if (!movie->GetVariable(&value, path) || value.GetType() != GFxValue::kType_Number) return false;
                result = value.GetNumber(); return true;
            }
            static void Write(Movie* movie, const char* path, double number) {
                GFxValue value; value.SetNumber(number); movie->SetVariable(path, &value, 0);
            }
        };
        struct HUDBoolAccess {
            using Movie = GFxMovieView;
            static bool Read(Movie* movie, const char* path, double& result) {
                GFxValue value;
                if (!movie->GetVariable(&value, path) || value.GetType() != GFxValue::kType_Bool) return false;
                result = value.GetBool() ? 1 : 0; return true;
            }
            static void Write(Movie* movie, const char* path, double enabled) {
                GFxValue value; value.SetBool(enabled != 0); movie->SetVariable(path, &value, 0);
            }
        };
        // SKSE exposes ProcessUnkData1 publicly but its parameter type privately.
        // Deduce that exact type; do not redeclare or modify the external SDK.
        template<class> struct MenuMessageArgument;
        template<class R, class C, class A> struct MenuMessageArgument<R(C::*)(A)> { using type = A; };
        using MenuMessagePointer = MenuMessageArgument<decltype(&IMenu::ProcessUnkData1)>::type;
        struct UIRegressionAccess;
        class TrueHUDMenu final : public IMenu {
            friend struct UIRegressionAccess;
            CustomHost customHost{view};
            CustomWidgetRegistry<CustomHost> custom{customHost, customDefinitions};
            Widget widgets[MaxActors];
            FloatingWidget floatingWidgets[MaxFloating];
            UInt32 lastFloatingSerial = 0;
            GFxValue playerWidget, lootWidget, shoutWidget;
            DisplayMemo playerPosition, lootPosition, shoutPosition;
            bool shoutConfigured = false;
            double shoutOpacity = 0;
            GFxValue depthArray;
            bool depthTopologyChanged = true;
            bool depthOrderDirty = true;
            unsigned depthExtras = 0;
            Frame frame;
            UInt32 lastGeneration = 0;
            UInt32 lastLootSerial = 0;
            Settings previous;
            bool settingsKnown = false;
            bool playerSpecialAvailable = false;
            UInt32 resourceRevision = 0;
            UInt32 playerColorRevision = 0;
            ColorRevisionMemo playerColorMemo;
            float voiceCooldown = 0;
            float shoutProgress = 1;
            HUDNumberOverride<HUDBoolAccess> healthVisible{"_root.HUDMovieBaseInstance.Health._visible"};
            HUDNumberOverride<HUDBoolAccess> magickaVisible{"_root.HUDMovieBaseInstance.Magica._visible"};
            HUDNumberOverride<HUDBoolAccess> staminaVisible{"_root.HUDMovieBaseInstance.Stamina._visible"};
            HUDNumberOverride<HUDBoolAccess> chargeLeftVisible{"_root.HUDMovieBaseInstance.BottomLeftLockInstance._visible"};
            HUDNumberOverride<HUDBoolAccess> chargeRightVisible{"_root.HUDMovieBaseInstance.BottomRightLockInstance._visible"};
            ChargeSample previousCharges[2];
            bool chargesKnown = false;
            HUDNumberOverride<HUDNumberAccess> targetAlpha{"_root.HUDMovieBaseInstance.EnemyHealth_mc._alpha"};
            HUDNumberOverride<HUDNumberAccess> compassAlpha{"_root.HUDMovieBaseInstance.CompassShoutMeterHolder._alpha"};
            HUDNumberOverride<HUDNumberAccess> subtitleY{"_root.HUDMovieBaseInstance.SubtitleTextHolder._y"};
            unsigned visibleBosses = 0;
            Timing timing{"UI"}; // phases: native UI work, movie advance, movie render
            bool disabledCleared = false;
            HUDSignals previousSignals;
            unsigned traceCount = 0;
            ULONGLONG nextTrace = 0;
            unsigned renderCount = 0;
            unsigned uiTickCount = 0, nextTraceTick = 0, lastTraceMode = 0;
            bool lastTraceReady = false;

            static int TraceNumber(double value) {
                return std::isfinite(value) && value >= -10000000 && value <= 10000000 ? int(value) : -1;
            }

            void TraceUI(unsigned mode, bool hudOpen, bool showMenus, bool cart, bool visibilityCall) {
                if (!LoggingEnabled()) return; // skip diagnostic GFx probes as well as disk output
                const ULONGLONG now = GetTickCount64();
                if (traceCount >= 32) return;
                if (traceCount && frame.ready == lastTraceReady && mode == lastTraceMode &&
                    uiTickCount < nextTraceTick && now < nextTrace) return;
                ++traceCount; nextTrace = now + 10000; nextTraceTick = uiTickCount + 600;
                lastTraceReady = frame.ready; lastTraceMode = mode;
                GFxValue root, mainURL, partialURL, mainFrames, partialFrames, rootVisible, alpha;
                const bool rootFound = view->GetVariable(&root, "_root.TrueHUD");
                view->GetVariable(&mainURL, "_root.TrueHUD.TrueHUD_MainVisibilityWidgets._url");
                view->GetVariable(&partialURL, "_root.TrueHUD.TrueHUD_PartialVisibilityWidgets._url");
                view->GetVariable(&mainFrames, "_root.TrueHUD.TrueHUD_MainVisibilityWidgets._framesloaded");
                view->GetVariable(&partialFrames, "_root.TrueHUD.TrueHUD_PartialVisibilityWidgets._framesloaded");
                view->GetVariable(&rootVisible, "_root.TrueHUD._visible");
                view->GetVariable(&alpha, "_root.TrueHUD._alpha");
                unsigned actorWidgets = 0;
                for (auto& widget : widgets) if (widget.object.IsDisplayObject()) ++actorWidgets;
                Log("UI probe %u: generation=%u ready=%u player=%08X actors=%u target=%08X mode=%u HUD-open=%u showMenus=%u cart=%u visibility-call=%u renders=%u",
                    traceCount, frame.generation, frame.ready, frame.player.handle, frame.count, frame.target,
                    mode, hudOpen, showMenus, cart, visibilityCall, renderCount);
                Log("UI movie: root-found=%u root-type=%u root-visible=%d root-alpha=%d main-frames=%d partial-frames=%d player-type=%u actor-widgets=%u loot-type=%u",
                    rootFound, root.GetType(), rootVisible.GetType() == GFxValue::kType_Bool ? int(rootVisible.GetBool()) : -1,
                    alpha.GetType() == GFxValue::kType_Number ? TraceNumber(alpha.GetNumber()) : -1,
                    mainFrames.GetType() == GFxValue::kType_Number ? TraceNumber(mainFrames.GetNumber()) : -1,
                    partialFrames.GetType() == GFxValue::kType_Number ? TraceNumber(partialFrames.GetNumber()) : -1,
                    playerWidget.GetType(), actorWidgets, lootWidget.GetType());
                Log("UI libraries: main=%s partial=%s", mainURL.GetType() == GFxValue::kType_String ? mainURL.GetString() : "<unavailable>",
                    partialURL.GetType() == GFxValue::kType_String ? partialURL.GetString() : "<unavailable>");
                const GRectF rect = view->GetVisibleFrameRect();
                Log("UI viewport: left=%d top=%d right=%d bottom=%d HUD-opacity-percent=%d",
                    TraceNumber(rect.left), TraceNumber(rect.top), TraceNumber(rect.right), TraceNumber(rect.bottom),
                    TraceNumber(*reinterpret_cast<const float*>(0x012B6304) * 100));
                Settings settings; CopySettings(settings);
                Log("UI settings: actors=%u bosses=%u player=%u loot=%u actor-scale-percent=%d boss-scale-percent=%d player-scale-percent=%d",
                    settings.bEnableActorInfoBars != 0, settings.bEnableBossBars != 0, settings.bEnablePlayerWidget != 0, settings.bEnableRecentLoot != 0,
                    TraceNumber(settings.fInfoBarScale * 100), TraceNumber(settings.fBossBarScale * 100),
                    TraceNumber(settings.fPlayerWidgetScale * 100));
            }

            GFxMovieView* NativeHUD() {
                auto manager = MenuManager::GetSingleton();
                return manager ? manager->GetMovieView(&hudName) : nullptr;
            }
            void NativeTarget() {
                auto manager = MenuManager::GetSingleton();
                auto hud = manager ? static_cast<HUDMenu*>(manager->GetMenu(&hudName)) : nullptr;
                UInt32 handle;
                const bool available = ReadNativeTarget(hud, handle);
                SetNativeTarget(handle, available);
            }

            void Remove(Widget& widget) {
                if (!widget.handle) return;
                if (widget.object.IsDisplayObject()) {
                    widget.object.Invoke("cleanUp", nullptr, nullptr, 0);
                    GFxValue arg;
                    arg.SetNumber(widget.handle);
                    view->Invoke(widget.boss ? "_root.TrueHUD.RemoveBossInfoBarWidget" :
                                 "_root.TrueHUD.RemoveInfoBarWidget", nullptr, &arg, 1);
                }
                widget.object.SetUndefined();
                widget.depthRow.SetUndefined();
                widget.handle = 0;
                widget.configured = false;
                widget.removal = 255;
                widget.colorRevision = 0;
                widget.colorMemo = {};
                widget.position.Reset();
                depthTopologyChanged = true;
            }
            void RestoreBars() {
                auto hud = NativeHUD();
                healthVisible.Restore(hud); magickaVisible.Restore(hud); staminaVisible.Restore(hud);
            }
            void RestoreVanilla() {
                RestoreBars();
                auto hud = NativeHUD();
                targetAlpha.Restore(hud); compassAlpha.Restore(hud); subtitleY.Restore(hud);
                chargeLeftVisible.Restore(hud); chargeRightVisible.Restore(hud);
            }
            void HideVanilla(bool force) {
                auto hud = NativeHUD();
                healthVisible.Apply(hud, true, 0, false, force);
                magickaVisible.Apply(hud, true, 0, false, force);
                staminaVisible.Apply(hud, true, 0, false, force);
            }
            void ChargeMeters(const Settings& settings) {
                auto hud = NativeHUD();
                const bool enabled = settings.bPlayerWidgetDisplayEnchantmentChargeMeter != 0 && playerWidget.IsDisplayObject();
                chargeLeftVisible.Apply(hud, enabled, 0, false, settings.bPlayerWidgetForceHideVanillaBars != 0);
                chargeRightVisible.Apply(hud, enabled, 0, false, settings.bPlayerWidgetForceHideVanillaBars != 0);
                if (!enabled) { chargesKnown = false; return; }
                ChargeSample charges[2]; ReadNativeCharges(charges);
                for (unsigned i = 0; i < 2; ++i) {
                    if (chargesKnown && settingsKnown && charges[i].percent == previousCharges[i].percent &&
                        charges[i].show == previousCharges[i].show) continue;
                    GFxValue args[4];
                    args[0].SetNumber(charges[i].percent); args[1].SetBool(!chargesKnown || !settingsKnown);
                    args[2].SetBool(i == 0); args[3].SetBool(charges[i].show);
                    playerWidget.Invoke("updateEnchantmentChargeMeter", nullptr, args, 4);
                    previousCharges[i] = charges[i];
                }
                chargesKnown = true;
            }
            void Clear(bool restoreNative = true) {
                SetLootReplacementReady(false);
                GFxValue hidden; hidden.SetBool(false);
                if (view) view->SetVariable("_root.TrueHUD._visible",&hidden,0);
                custom.Clear(); customHost.ClearDepths();
                ResetAPIColors(); PublishInfoBars(nullptr, nullptr, 0);
                if (restoreNative) RestoreVanilla();
                if (!view) return;
                for (auto& widget : widgets) Remove(widget);
                for (auto& floatingWidget : floatingWidgets) RemoveFloating(floatingWidget);
                lastFloatingSerial = 0;
                if (playerWidget.IsDisplayObject()) {
                    playerWidget.Invoke("cleanUp", nullptr, nullptr, 0);
                    view->Invoke("_root.TrueHUD.RemovePlayerWidget", nullptr, static_cast<GFxValue*>(nullptr), 0);
                }
                if (lootWidget.IsDisplayObject()) {
                    lootWidget.Invoke("cleanUp", nullptr, nullptr, 0);
                    view->Invoke("_root.TrueHUD.RemoveRecentLootWidget", nullptr, static_cast<GFxValue*>(nullptr), 0);
                }
                if (shoutWidget.IsDisplayObject()) {
                    shoutWidget.Invoke("cleanUp", nullptr, nullptr, 0);
                    view->Invoke("_root.TrueHUD.RemoveShoutIndicatorWidget", nullptr, static_cast<GFxValue*>(nullptr), 0);
                }
                playerWidget.SetUndefined(); lootWidget.SetUndefined(); shoutWidget.SetUndefined();
                playerPosition.Reset(); lootPosition.Reset(); shoutPosition.Reset(); shoutConfigured = false;
                playerColorMemo = {};
                depthArray.SetUndefined(); depthTopologyChanged = true; depthOrderDirty = true; depthExtras = 0;
                settingsKnown = false;
                voiceCooldown = 0;
                shoutProgress = 1;
                lastLootSerial = 0;
                chargesKnown = false;
                previousSignals = {};
            }
            void Loot(const Settings& settings, const GRectF& rect, float opacity) {
                SetLootReplacementReady(false);
                if (!settings.bEnableRecentLoot) {
                    if (lootWidget.IsDisplayObject()) {
                        lootWidget.Invoke("cleanUp", nullptr, nullptr, 0);
                        view->Invoke("_root.TrueHUD.RemoveRecentLootWidget", nullptr, static_cast<GFxValue*>(nullptr), 0);
                        lootWidget.SetUndefined();
                    }
                    if (frame.lootCount) lastLootSerial = frame.loot[frame.lootCount - 1].serial;
                    return;
                }
                const bool newlyCreated = !lootWidget.IsDisplayObject();
                if (newlyCreated) { lootPosition.Reset(); view->Invoke("_root.TrueHUD.AddRecentLootWidget", &lootWidget, static_cast<GFxValue*>(nullptr), 0); }
                if (!lootWidget.IsDisplayObject()) return;
                if (!settingsKnown || newlyCreated)
                    ConfigureRecentLoot(lootWidget, settings, opacity, rect.right - rect.left, rect.bottom - rect.top);
                Position(lootWidget, rect.left + (rect.right - rect.left) * settings.fRecentLootX,
                         rect.top + (rect.bottom - rect.top) * settings.fRecentLootY, settings.fRecentLootScale * 100, true, &lootPosition);
                for (unsigned i = 0; i < frame.lootCount; ++i) {
                    const auto& item = frame.loot[i];
                    if (item.serial <= lastLootSerial) continue;
                    GFxValue args[4];
                    args[0].SetString(item.name); args[1].SetNumber(item.count);
                    args[2].SetString(item.icon); args[3].SetNumber(item.iconColor);
                    lootWidget.Invoke("addMessage", nullptr, args, 4);
                    lastLootSerial = item.serial;
                }
                lootWidget.Invoke("update", nullptr, nullptr, 0);
                SetLootReplacementReady(true);
            }
            void Player(const Settings& settings, const GRectF& rect, float opacity) {
                if (!settings.bEnablePlayerWidget) {
                    if (playerWidget.IsDisplayObject()) {
                        RestoreBars();
                        view->Invoke("_root.TrueHUD.RemovePlayerWidget", nullptr, static_cast<GFxValue*>(nullptr), 0);
                        playerWidget.SetUndefined();
                    }
                    return;
                }
                if (!playerWidget.IsDisplayObject()) {
                    playerPosition.Reset();
                    view->Invoke("_root.TrueHUD.AddPlayerWidget", &playerWidget, static_cast<GFxValue*>(nullptr), 0);
                }
                if (!playerWidget.IsDisplayObject()) return; // submovie still loading: retry later
                const UInt32 revision = ReadAPIColorRevision(frame.player.handle, playerColorMemo);
                if (!settingsKnown || playerSpecialAvailable != frame.player.special.available || revision != playerColorRevision) {
                    Settings colored = settings;
                    CopyAPIColors(frame.player.handle, colored);
                    ConfigurePlayerWidget(playerWidget, colored, opacity, rect.right - rect.left, rect.bottom - rect.top,
                                          frame.player.special.specialMode, frame.player.special.available);
                    playerSpecialAvailable = frame.player.special.available;
                    playerColorRevision = revision;
                    GFxValue penalty[4];
                    for (unsigned i = 0; i < 3; ++i) penalty[i].SetNumber(0);
                    penalty[3].SetBool(false);
                    playerWidget.Invoke("updatePenalty", nullptr, penalty, 4);
                }
                Position(playerWidget, settings.bPlayerWidgetCombined ? rect.left + (rect.right - rect.left) * settings.fPlayerWidgetX : 0,
                         settings.bPlayerWidgetCombined ? rect.top + (rect.bottom - rect.top) * settings.fPlayerWidgetY : 0,
                         settings.fPlayerWidgetScale * 100, true, &playerPosition);
                GFxValue data[12];
                const bool mountMode = settings.bPlayerWidgetDisplayMountStamina != 0 && frame.player.mounted;
                for (unsigned i = 0; i < 3; ++i) {
                    const float current = i == 2 && mountMode ? frame.player.mountStamina : frame.player.current[i];
                    const float maximum = i == 2 && mountMode ? frame.player.mountMaximumStamina : frame.player.maximum[i];
                    data[i * 3].SetNumber(std::ceil(maximum));
                    data[i * 3 + 1].SetNumber(std::ceil(current));
                    data[i * 3 + 2].SetNumber(Percent(current, maximum));
                }
                data[9].SetBool(frame.player.combat); data[10].SetBool(mountMode); data[11].SetBool(false);
                playerWidget.Invoke("updateData", nullptr, data, 12);
                GFxValue special[3];
                special[0].SetNumber(std::ceil(frame.player.special.maximum));
                special[1].SetNumber(std::ceil(frame.player.special.current));
                special[2].SetNumber(Percent(frame.player.special.current, frame.player.special.maximum));
                playerWidget.Invoke("updateSpecial", nullptr, special, 3);
                GFxValue progress;
                progress.SetNumber(shoutProgress);
                playerWidget.Invoke("setProgress", nullptr, &progress, 1);
                if (settings.bPlayerWidgetHideVanillaBars) HideVanilla(settings.bPlayerWidgetForceHideVanillaBars != 0); else RestoreBars();
            }
            void RemoveFloating(FloatingWidget& floatingWidget) {
                if (!floatingWidget.serial) return;
                GFxValue::DisplayInfo info;
                if (floatingWidget.object.IsDisplayObject() && floatingWidget.object.GetDisplayInfo(&info))
                    floatingWidget.object.Invoke("dispose",nullptr,nullptr,0);
                floatingWidget.object.SetUndefined(); floatingWidget.depthRow.SetUndefined();
                floatingWidget.serial = 0; floatingWidget.age = floatingWidget.velocity = 0;
                depthTopologyChanged = true;
            }
            void FloatingText(const Settings& settings, const GRectF& rect, float delta) {
                const bool enabled = settings.bEnableFloatingText && settings.bEnableFloatingCombatText;
                if (!enabled) {
                    for (auto& floatingWidget : floatingWidgets) RemoveFloating(floatingWidget);
                }
                for (unsigned i=0;i<frame.floatingCount;++i) {
                    const auto& event=frame.floating[i];
                    if (static_cast<SInt32>(event.serial-lastFloatingSerial)<=0) continue;
                    lastFloatingSerial=event.serial;
                    if (!enabled) continue;
                    FloatingWidget* selected=nullptr;
                    for (auto& floatingWidget : floatingWidgets) if (!floatingWidget.serial) { selected=&floatingWidget; break; }
                    if (!selected) {
                        selected=&floatingWidgets[0];
                        for (auto& floatingWidget : floatingWidgets) if (floatingWidget.age>selected->age) selected=&floatingWidget;
                        RemoveFloating(*selected);
                    }
                    selected->serial=event.serial; selected->color=event.color; selected->position=event.position;
                    sprintf_s(selected->text,"%lu",static_cast<UInt32>(std::ceil(event.amount)));
                }
                for (unsigned i=0;i<MaxFloating;++i) {
                    auto& floatingWidget=floatingWidgets[i];
                    if (!floatingWidget.serial) continue;
                    floatingWidget.age += delta;
                    if (!floatingWidget.object.IsDisplayObject()) {
                        if (floatingWidget.age>=5) { RemoveFloating(floatingWidget); continue; }
                        GFxValue id; id.SetNumber(i+1);
                        view->Invoke("_root.TrueHUD.AddFloatingTextWidget",&floatingWidget.object,&id,1);
                        if (!floatingWidget.object.IsDisplayObject()) continue;
                        GFxValue config[4]; config[0].SetString(floatingWidget.text); config[1].SetNumber(floatingWidget.color);
                        config[2].SetNumber(2); config[3].SetBool(false);
                        floatingWidget.object.Invoke("loadConfig",nullptr,config,4);
                        floatingWidget.age=0; depthTopologyChanged=true;
                    } else {
                        GFxValue::DisplayInfo info;
                        if (!floatingWidget.object.GetDisplayInfo(&info)) { RemoveFloating(floatingWidget); continue; }
                        if (floatingWidget.age>=2.25f) { RemoveFloating(floatingWidget); continue; }
                    }
                    floatingWidget.velocity *= 1-delta;
                    floatingWidget.velocity -= 120*delta;
                    floatingWidget.position.z += floatingWidget.velocity*delta;
                    float x,y,depth,distance;
                    const bool projected=Project(floatingWidget.position,x,y,depth,&distance);
                    Position(floatingWidget.object,rect.left+(rect.right-rect.left)*x,rect.top+(rect.bottom-rect.top)*(1-y),
                        DistanceScale(distance,settings.bFloatingTextScaleWithDistance!=0,settings.fFloatingTextScale),projected && depth>0);
                    floatingWidget.depth=depth;
                }
            }
            void Actors(const Settings& settings, const GRectF& rect, float opacity) {
                visibleBosses = 0;
                for (auto& widget : widgets) {
                    if (!widget.handle) continue;
                    bool found = false;
                    for (unsigned i = 0; i < frame.count; ++i)
                        if (frame.actors[i].handle == widget.handle) { found = true; break; }
                    if (!found || (widget.boss ? !settings.bEnableBossBars : !settings.bEnableActorInfoBars)) Remove(widget);
                }
                if (!settings.bEnableActorInfoBars && !settings.bEnableBossBars) return;
                unsigned bossIndex = 0;
                for (unsigned i = 0; i < frame.count; ++i) {
                    const ActorSample& sample = frame.actors[i];
                    Widget* existing = nullptr;
                    for (auto& widget : widgets) if (widget.handle == sample.handle) { existing = &widget; break; }
                    if (sample.removal == 0 || (sample.removal != 255 && !existing)) {
                        if (existing) Remove(*existing);
                        ForgetActor(sample.handle); continue;
                    }
                    const bool defeated = sample.dead || (sample.bleeding && sample.essential);
                    const bool boss = settings.bEnableBossBars && (sample.boss || (defeated && existing && existing->boss));
                    if ((!boss && !settings.bEnableActorInfoBars) ||
                        (boss && bossIndex >= settings.uBossBarMaxCount)) {
                        // A settings/role change must retire an existing widget,
                        // not leave a previously admitted bar visible forever.
                        if (existing) Remove(*existing);
                        continue;
                    }
                    const unsigned type = sample.handle == frame.target ? 0 : sample.teammate ? 2 : sample.hostile ? 1 : 3;
                    const double mode = type == 1 ? settings.uInfoBarDisplayHostiles : type == 2 ?
                        settings.uInfoBarDisplayTeammates : settings.uInfoBarDisplayOthers;
                    if (!boss && !sample.manualRole && type && (mode == 0 || (mode == 1 && !sample.hit))) {
                        if (existing) Remove(*existing);
                        if (sample.dead) ForgetActor(sample.handle);
                        continue;
                    }
                    Widget* selected = existing;
                    if (!selected) for (auto& widget : widgets) if (!widget.handle) { selected = &widget; break; }
                    if (!selected) break;
                    Widget& widget = *selected;
                    if (widget.handle && widget.boss != boss && !sample.dead) Remove(widget);
                    if (!widget.handle) {
                        GFxValue id;
                        id.SetNumber(sample.handle);
                        view->Invoke(boss ? "_root.TrueHUD.AddBossInfoBarWidget" : "_root.TrueHUD.AddInfoBarWidget", &widget.object, &id, 1);
                        if (!widget.object.IsDisplayObject()) continue;
                        widget.handle = sample.handle;
                        widget.boss = boss;
                        depthTopologyChanged = true;
                    }
                    GFxValue removed;
                    widget.object.Invoke("isReadyToRemove", &removed, nullptr, 0);
                    if (removed.GetType() == GFxValue::kType_Bool && removed.GetBool()) {
                        ForgetActor(sample.handle);
                        Remove(widget);
                        continue;
                    }
                    const UInt32 revision = ReadAPIColorRevision(sample.handle, widget.colorMemo);
                    if (!widget.configured || !settingsKnown || widget.specialAvailable != sample.special.available || revision != widget.colorRevision) {
                        Settings colored = settings;
                        CopyAPIColors(sample.handle, colored);
                        if (widget.boss) ConfigureBossInfoBar(widget.object, colored, opacity, rect.right - rect.left, rect.bottom - rect.top,
                                                           sample.special.specialMode, sample.special.available);
                        else ConfigureActorInfoBar(widget.object, colored, opacity, rect.right - rect.left, rect.bottom - rect.top,
                                                   sample.special.specialMode, sample.special.available);
                        widget.configured = true;
                        widget.specialAvailable = sample.special.available;
                        widget.colorRevision = revision;
                    }
                    if (widget.boss) {
                        const double direction = settings.uMultipleBossBarsStackDirection ? 1 : -1;
                        Position(widget.object, rect.left + (rect.right - rect.left) * settings.fBossBarAnchorX,
                                 rect.top + (rect.bottom - rect.top) * settings.fBossBarAnchorY + direction * bossIndex * settings.fMultipleBossBarsOffset,
                                 settings.fBossBarScale * 100, true, &widget.position);
                        ++bossIndex;
                    } else {
                        Position(widget.object, rect.left + (rect.right - rect.left) * sample.screenX,
                                 rect.top + (rect.bottom - rect.top) * (1 - sample.screenY),
                                 DistanceScale(sample.cameraDistance, settings.bInfoBarScaleWithDistance != 0, settings.fInfoBarScale),
                                 sample.projected && sample.depth > 0 && sample.visible, &widget.position);
                    }
                    GFxValue data[18];
                    const unsigned removal = widget.removal != 255 ? widget.removal : sample.removal;
                    data[0].SetNumber(type); data[1].SetNumber(removal != 255 ? (removal == 2 ? 3 : 4) :
                        defeated ? 3 : (!widget.boss && !sample.visible) ? 1 : 0);
                    data[2].SetNumber(sample.depth); data[3].SetNumber(sample.dead ? 4 : sample.bleeding ? (sample.essential ? 3 : 2) : 0);
                    data[4].SetString(sample.name); data[5].SetNumber(sample.level);
                    double color = settings.uDefaultColor, outline = settings.uDefaultColorOutline;
                    const double threshold = widget.boss ? settings.uBossBarLevelThreshold : settings.uInfoBarLevelThreshold;
                    const bool coloredOutline = widget.boss ? settings.bBossBarLevelColorOutline != 0 : settings.bInfoBarLevelColorOutline != 0;
                    if (sample.teammate) { color = settings.uTeammateColor; outline = settings.uTeammateColorOutline; }
                    else if (int(sample.level) - int(frame.player.level) > threshold) {
                        color = settings.uStrongerColor; if (coloredOutline) outline = settings.uStrongerColorOutline;
                    } else if (int(frame.player.level) - int(sample.level) > threshold) {
                        color = settings.uWeakerColor; if (coloredOutline) outline = settings.uWeakerColorOutline;
                    }
                    data[6].SetNumber(color); data[7].SetNumber(outline); data[8].SetNumber(sample.soul);
                    for (unsigned av = 0; av < 3; ++av) {
                        const float value = sample.dead ? 0 : sample.current[av];
                        data[9 + av * 3].SetNumber(std::ceil(sample.maximum[av]));
                        data[10 + av * 3].SetNumber(std::ceil(value));
                        data[11 + av * 3].SetNumber(Percent(value, sample.maximum[av]));
                    }
                    widget.object.Invoke("updateData", nullptr, data, 18);
                    GFxValue special[3];
                    special[0].SetNumber(std::ceil(sample.special.maximum));
                    special[1].SetNumber(std::ceil(sample.special.current));
                    special[2].SetNumber(Percent(sample.special.current, sample.special.maximum));
                    widget.object.Invoke("updateSpecial", nullptr, special, 3);
                    widget.depth = widget.boss ? 0.f : sample.depth;
                }
                visibleBosses = bossIndex;
                UInt32 infoHandles[MaxActors]; bool infoBosses[MaxActors]; unsigned count = 0;
                for (const auto& widget : widgets) if (widget.handle && widget.object.IsDisplayObject()) {
                    infoHandles[count] = widget.handle; infoBosses[count++] = widget.boss;
                }
                PublishInfoBars(infoHandles, infoBosses, count);
            }
            void SortDepths() {
                const unsigned extras = (shoutWidget.IsDisplayObject() ? 1 : 0) |
                    (playerWidget.IsDisplayObject() ? 2 : 0) | (lootWidget.IsDisplayObject() ? 4 : 0);
                if (depthTopologyChanged || extras != depthExtras) {
                    depthArray.SetUndefined();
                    view->CreateArray(&depthArray);
                    if (depthArray.GetType() != GFxValue::kType_Array) return;
                    auto append = [&](GFxValue& row, UInt32 id, unsigned type, double depth) {
                        row.SetUndefined(); view->CreateObject(&row);
                        if (!row.IsObject()) return false;
                        GFxValue value;
                        value.SetNumber(id); if (!row.SetMember("id", &value)) return false;
                        value.SetNumber(type); if (!row.SetMember("widgetType", &value)) return false;
                        value.SetNumber(depth); if (!row.SetMember("zIndex", &value)) return false;
                        return depthArray.PushBack(&row);
                    };
                    bool complete = true;
                    for (auto& widget : widgets) {
                        if (widget.handle && widget.object.IsDisplayObject())
                            complete = append(widget.depthRow, widget.handle, widget.boss ? 1 : 0, widget.depth) && complete;
                    }
                    for (unsigned i=0;i<MaxFloating;++i) {
                        auto& floatingWidget=floatingWidgets[i];
                        if (floatingWidget.serial && floatingWidget.object.IsDisplayObject())
                            complete = append(floatingWidget.depthRow,i+1,5,floatingWidget.depth) && complete;
                    }
                    GFxValue row;
                    if (extras & 1) complete = append(row, 0, 2, 0) && complete;
                    if (extras & 2) complete = append(row, 0, 3, 0) && complete;
                    if (extras & 4) complete = append(row, 0, 4, 0) && complete;
                    if (!complete) return; // Retry incomplete movie allocation on a later UI frame.
                    depthExtras = extras; depthTopologyChanged = false;
                    for (auto& widget : widgets) widget.sortedDepth = widget.depth;
                    for (auto& floatingWidget : floatingWidgets) floatingWidget.sortedDepth = floatingWidget.depth;
                    depthOrderDirty = true;
                } else {
                    // Existing numeric properties need no new array/row allocation.
                    for (auto& widget : widgets) if (widget.handle && widget.object.IsDisplayObject() && widget.sortedDepth != widget.depth) {
                        GFxValue value; value.SetNumber(widget.depth);
                        if (!widget.depthRow.SetMember("zIndex", &value)) return;
                        widget.sortedDepth = widget.depth; depthOrderDirty = true;
                    }
                    for (auto& floatingWidget : floatingWidgets) if (floatingWidget.serial && floatingWidget.object.IsDisplayObject() && floatingWidget.sortedDepth != floatingWidget.depth) {
                        GFxValue value; value.SetNumber(floatingWidget.depth);
                        if (!floatingWidget.depthRow.SetMember("zIndex",&value)) return;
                        floatingWidget.sortedDepth = floatingWidget.depth; depthOrderDirty = true;
                    }
                }
                // Preserve the supplied movie's own descending zIndex policy,
                // including relative order of boss/player/loot widgets.
                if (depthOrderDirty && view->Invoke("_root.TrueHUD.SortDepths", nullptr, &depthArray, 1)) depthOrderDirty = false;
            }
            void ModifyHUD(const Settings& settings, const GRectF& rect, float opacity) {
                auto hud = NativeHUD();
                const bool hideCompass = visibleBosses && settings.uBossBarModifyHUD == 2;
                targetAlpha.Apply(hud, settings.bHideVanillaTargetBar || hideCompass, 0);
                compassAlpha.Apply(hud, hideCompass, 0);
                subtitleY.Apply(hud, visibleBosses && settings.uBossBarModifyHUD == 1,
                    -settings.fMultipleBossBarsOffset * visibleBosses, true);
                const bool standalone = hideCompass && settings.bDisplayStandaloneShoutWidgetWhenHidingCompass &&
                    (!settings.bEnablePlayerWidget || settings.uPlayerWidgetShoutIndicatorMode == 0);
                if (standalone && !shoutWidget.IsDisplayObject())
                {
                    shoutConfigured = false; shoutPosition.Reset();
                    view->Invoke("_root.TrueHUD.AddShoutIndicatorWidget", &shoutWidget, static_cast<GFxValue*>(nullptr), 0);
                }
                if (!standalone && shoutWidget.IsDisplayObject()) {
                    shoutWidget.Invoke("cleanUp", nullptr, nullptr, 0);
                    view->Invoke("_root.TrueHUD.RemoveShoutIndicatorWidget", nullptr, static_cast<GFxValue*>(nullptr), 0);
                    shoutWidget.SetUndefined();
                    shoutConfigured = false; shoutPosition.Reset();
                }
                if (standalone && shoutWidget.IsDisplayObject()) {
                    const double alpha = (settings.bBossBarUseHUDOpacity ? opacity : settings.fBossBarOpacity) * 100.f;
                    if (!shoutConfigured || shoutOpacity != alpha) {
                        shoutConfigured = ConfigureShoutIndicator(shoutWidget, settings, opacity, rect.right - rect.left, rect.bottom - rect.top);
                        if (shoutConfigured) shoutOpacity = alpha;
                    }
                    Position(shoutWidget, rect.left + (rect.right - rect.left) * settings.fBossBarAnchorX + settings.fBossBarWidth * .5 + 50,
                        rect.top + (rect.bottom - rect.top) * settings.fBossBarAnchorY, 50, true, &shoutPosition);
                    GFxValue progress;
                    progress.SetNumber(shoutProgress);
                    shoutWidget.Invoke("setProgress", nullptr, &progress, 1);
                }
            }
            void Flash(const HUDSignals& current) {
                for (unsigned i = 0; i < 4; ++i) {
                    if (current.flashes[i] == previousSignals.flashes[i]) continue;
                    if (playerWidget.IsDisplayObject()) {
                        GFxValue args[2]; args[0].SetNumber(i < 3 ? 24 + i : 0x26); args[1].SetBool(true);
                        playerWidget.Invoke("flashAV", nullptr, args, 2);
                    }
                    if (i == 3) {
                        GFxValue value; value.SetBool(true);
                        if (shoutWidget.IsDisplayObject()) shoutWidget.Invoke("playFlash", nullptr, &value, 1);
                    }
                }
                previousSignals = current;
            }
            void DispatchCommands() {
                custom.RestoreDefinitions();
                APICommand command; bool current = false;
                for (unsigned budget = 0; budget < 64 && PopAPICommand(command, current); ++budget) {
                    if (!current) continue;
                    if (custom.Process(command)) continue;
                    if (command.kind == APICommandKind::OverrideColor || command.kind == APICommandKind::RevertColor) {
                        ProcessAPIColor(command); continue;
                    }
                    const bool boss = command.kind == APICommandKind::AddBoss || command.kind == APICommandKind::RemoveBoss;
                    const bool remove = command.kind == APICommandKind::RemoveActor || command.kind == APICommandKind::RemoveBoss;
                    if (command.kind == APICommandKind::AddActor || command.kind == APICommandKind::AddBoss || remove) {
                        Widget* existing = nullptr;
                        for (auto& widget : widgets) if (widget.handle == command.actor) { existing = &widget; break; }
                        if (remove && (!existing || existing->boss != boss)) { if (!existing) ForgetActor(command.actor); continue; }
                        if (remove && !command.value) { Remove(*existing); ForgetActor(command.actor); continue; }
                        if (existing) existing->removal = remove ? static_cast<UInt8>(command.value) : 255;
                        ApplyActorCommand(command.actor, boss, remove, command.value);
                        continue;
                    }
                    if (command.kind == APICommandKind::FlashActor || command.kind == APICommandKind::FlashSpecial) {
                        const bool special = command.kind == APICommandKind::FlashSpecial;
                        if (special && ReadSpecialStatus().owner != command.owner) continue;
                        GFxValue args[2]; args[0].SetNumber(command.value); args[1].SetBool(command.longFlash);
                        GFxValue flash; flash.SetBool(command.longFlash);
                        if (command.actor == frame.player.handle && playerWidget.IsDisplayObject()) {
                            if (special) playerWidget.Invoke("flashSpecial", nullptr, &flash, 1);
                            else playerWidget.Invoke("flashAV", nullptr, args, 2);
                        }
                        for (auto& widget : widgets) if (widget.handle == command.actor) {
                            if (special) widget.object.Invoke("flashSpecial", nullptr, &flash, 1);
                            else widget.object.Invoke("flashAV", nullptr, args, 2);
                        }
                    }
                }
            }
            void DrawDebug(const GRectF& rect) {
                const unsigned count = CopyDebugDrawing(debugScratch, MaxDebugLines, GetTickCount64());
                const float width = rect.right-rect.left, height = rect.bottom-rect.top;
                for (unsigned i=0;i<count;++i) {
                    const auto& line=debugScratch[i]; float x,y,z,ex,ey,ez;
                    if (!Project(line.start,x,y,z) || z<=0) continue;
                    x=rect.left+x*width; y=rect.top+(1-y)*height;
                    if (line.pointSize>0) {
                        if (x<rect.left || x>rect.right || y<rect.top || y>rect.bottom) continue;
                        GFxValue fill[2]; fill[0].SetNumber(line.color>>8); fill[1].SetNumber((line.color&255)*100.0/255);
                        view->Invoke("beginFill",nullptr,fill,2);
                        GFxValue point[4]; point[0].SetNumber(x+line.pointSize); point[1].SetNumber(y);
                        view->Invoke("moveTo",nullptr,point,2);
                        constexpr float pi=3.14159265358979323846f;
                        const float control=line.pointSize/std::cos(pi/8);
                        for (unsigned j=1;j<=8;++j) {
                            const float angle=j*pi/4;
                            point[0].SetNumber(x+std::cos(angle-pi/8)*control);
                            point[1].SetNumber(y+std::sin(angle-pi/8)*control);
                            point[2].SetNumber(x+std::cos(angle)*line.pointSize);
                            point[3].SetNumber(y+std::sin(angle)*line.pointSize);
                            view->Invoke("curveTo",nullptr,point,4);
                        }
                        view->Invoke("endFill",nullptr,nullptr,0); continue;
                    }
                    if (!Project(line.end,ex,ey,ez) || ez<=0) continue;
                    ex=rect.left+ex*width; ey=rect.top+(1-ey)*height;
                    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(ex) || !std::isfinite(ey)) continue;
                    // Clip the projected segment to the visible rectangle.
                    const float dx=ex-x, dy=ey-y;
                    const float p[]={-dx,dx,-dy,dy}, q[]={x-rect.left,rect.right-x,y-rect.top,rect.bottom-y};
                    float begin=0,end=1; bool accepted=true;
                    for (unsigned j=0;j<4;++j) {
                        if (p[j]==0) { if (q[j]<0) accepted=false; continue; }
                        const float t=q[j]/p[j];
                        if (p[j]<0) begin=std::max(begin,t); else end=std::min(end,t);
                        if (begin>end) accepted=false;
                    }
                    if (!accepted) continue;
                    GFxValue style[3]; style[0].SetNumber(line.thickness); style[1].SetNumber(line.color>>8);
                    style[2].SetNumber((line.color&255)*100.0/255);
                    view->Invoke("lineStyle",nullptr,style,3);
                    GFxValue point[2]; point[0].SetNumber(x+dx*begin); point[1].SetNumber(y+dy*begin);
                    view->Invoke("moveTo",nullptr,point,2);
                    point[0].SetNumber(x+dx*end); point[1].SetNumber(y+dy*end);
                    view->Invoke("lineTo",nullptr,point,2);
                }
            }
            void Tick(float delta) {
                if (!view) return;
                view->Invoke("clear", nullptr, static_cast<GFxValue*>(nullptr), 0);
                if (Disabled()) {
                    if (!disabledCleared) { disabledCleared = true; Clear(); }
                    return;
                }
                NativeTarget();
                if (!uiThread) { uiThread = GetCurrentThreadId(); SetAPIThread(uiThread); Log("menu NextFrame UI thread=%lu", uiThread); }
                CopyFrame(frame);
                if (lastGeneration != frame.generation) { Clear(); lastGeneration = frame.generation; }
                DispatchCommands();
                auto manager = MenuManager::GetSingleton();
                const bool hudOpen = manager && manager->IsMenuOpen(&hudName);
                const bool showMenus = manager && *reinterpret_cast<const bool*>(reinterpret_cast<const char*>(manager) + 0x118);
                bool visible = frame.ready && !Disabled() && hudOpen && showMenus;
                Settings settings;
                CopySettings(settings);
                HUDSignals signals;
                CopyHUDSignals(signals);
                visible = visible && !signals.cartMode;
                unsigned mode = visible ? 2 : 0;
                auto names = UIStringHolder::GetSingleton();
                if (visible && names && Readable(names, sizeof(UIStringHolder))) {
                    BSFixedString* blocked[] = {&names->mainMenu, &names->loadingMenu, &names->tweenMenu,
                        &names->magicMenu, &names->mapMenu, &names->journalMenu, &names->statsMenu,
                        &names->raceSexMenu, &names->sleepWaitMenu, &names->bookMenu, &names->trainingMenu};
                    for (auto name : blocked) if (manager->IsMenuOpen(name)) { mode = 0; break; }
                    BSFixedString* inventory[] = {&names->inventoryMenu, &names->barterMenu, &names->giftMenu, &names->containerMenu};
                    if (mode) for (auto name : inventory) if (manager->IsMenuOpen(name))
                        mode = settings.bRecentLootHideInInventoryMenus ? 0 : 1;
                    if (mode && manager->IsMenuOpen(&names->craftingMenu)) mode = settings.bRecentLootHideInCraftingMenus ? 0 : 1;
                    if (mode && manager->IsMenuOpen(&names->dialogueMenu)) mode = 1;
                }
                GFxValue visibility;
                visibility.SetNumber(mode);
                const bool visibilityCall = view->Invoke("_root.TrueHUD.SetVisibilityMode", nullptr, &visibility, 1);
                GFxValue rootVisible; rootVisible.SetBool(mode != 0);
                view->SetVariable("_root.TrueHUD._visible", &rootVisible, 0);
                if (!mode) { TraceUI(mode, hudOpen, showMenus, signals.cartMode, visibilityCall); SetLootReplacementReady(false); RestoreVanilla(); previousSignals = signals; RequestFrame(); return; }
                if (settingsKnown && memcmp(&settings, &previous, sizeof(Settings))) settingsKnown = false;
                if (resourceRevision != frame.player.special.revision) {
                    resourceRevision = frame.player.special.revision; settingsKnown = false;
                }
                const GRectF rect = view->GetVisibleFrameRect();
                const float opacity = *reinterpret_cast<const float*>(0x012B6304);
                shoutProgress = ShoutProgress(frame.player.voiceRecovery, voiceCooldown,
                    *reinterpret_cast<const float*>(0x01B3EAC8));
                Player(settings, rect, opacity);
                ChargeMeters(settings);
                Actors(settings, rect, opacity);
                ModifyHUD(settings, rect, opacity);
                Loot(settings, rect, opacity);
                FloatingText(settings,rect,delta);
                SortDepths();
                custom.Tick(delta); customHost.Sort(custom);
                DrawDebug(rect);
                Flash(signals);
                previous = settings;
                settingsKnown = playerWidget.IsDisplayObject() || !settings.bEnablePlayerWidget;
                TraceUI(mode, hudOpen, showMenus, signals.cartMode, visibilityCall);
                RequestFrame();
            }
        public:
            UInt32 ProcessUnkData1(MenuMessagePointer message) override {
                __try {
                    // Display-only overlay. The SDK default consumes type6
                    // events even when its movie is hidden, blocking lower menus.
                    return 2; // let the underlying menu receive input
                }
                __except(Fault("overlay message", GetExceptionInformation())) { return 2; }
            }
            TrueHUDMenu() {
                auto loader = GFxLoader::GetSingleton();
                if (loader) CALL_MEMBER_FN(loader, LoadMovie)(this, &view, "TrueHUD", 1, 0.f);
                flags = 0x00018902;
                unk0C = 2;
                Log("TrueHUD Test 7: menu created, movie=%p", view);
            }
            ~TrueHUDMenu() override {
                Log("TrueHUD menu destroying: ticks=%u renders=%u probes=%u", uiTickCount, renderCount, traceCount);
                SafeClear(false); // other menus may already be destroyed at process exit
                Log("TrueHUD menu owned clips cleared");
            }
            void NextFrame(UInt32 interval, UInt32 time) override {
                if (uiTickCount != 0xFFFFFFFFu) ++uiTickCount;
                if (uiTickCount <= 3) Log("TrueHUD frame callback: tick=%u time=%u thread=%lu", uiTickCount, time, GetCurrentThreadId());
                float delta; memcpy(&delta, &interval, sizeof(delta));
                if (!std::isfinite(delta) || delta < 0) delta = 0;
                delta = std::min(delta, .25f);
                __try {
                    ULONGLONG start = timing.Begin(); Tick(delta); timing.End(0, start, visibleBosses);
                    start = timing.Begin(); IMenu::NextFrame(interval, time); timing.End(1, start, visibleBosses);
                }
                __except(Fault("menu NextFrame", GetExceptionInformation())) { SafeClear(); }
            }
            void Render() override {
                __try {
                    if (!renderCount) Log("TrueHUD Render callback thread=%lu", GetCurrentThreadId());
                    if (renderCount != 0xFFFFFFFFu) ++renderCount;
                    const ULONGLONG start = timing.Begin(); IMenu::Render(); timing.End(2, start, visibleBosses, true);
                }
                __except(Fault("menu Render", GetExceptionInformation())) { SafeClear(); }
            }
            void SafeClear(bool restoreNative = true) {
                __try { Clear(restoreNative); }
                __except(Fault("menu restore", GetExceptionInformation())) {}
            }
        };
        IMenu* CreateMenuBody() {
            void* memory = ScaleformHeap_Allocate(sizeof(TrueHUDMenu));
            return memory ? new (memory) TrueHUDMenu() : nullptr;
        }
        IMenu* CreateMenu() {
            __try {
                return CreateMenuBody();
            }
            __except(Fault("menu creator", GetExceptionInformation())) { return nullptr; }
        }
        class OpenTask final : public UIDelegate_v1 {
        public:
            void Run() override {
                __try {
                    auto manager = MenuManager::GetSingleton();
                    auto ui = UIManager::GetSingleton();
                    if (!Disabled() && registered && manager && ui && !manager->IsMenuOpen(&menuName))
                        CALL_MEMBER_FN(ui, AddMessage)(&menuName, 1, nullptr);
                }
                __except(Fault("open UI task", GetExceptionInformation())) {}
            }
            void Dispose() override { InterlockedExchange(&openPending, 0); }
        } openTask;
    }
    void InitializeUI(SKSETaskInterface* tasks) { uiTasks = tasks; }
    void OnNativeHUDFrame() {
        if (!registered || Disabled()) return;
        auto manager = MenuManager::GetSingleton();
        if (!manager || !manager->IsMenuOpen(&hudName)) return;
        if (!manager->IsMenuOpen(&menuName)) {
            RequestMenuOpen();
            RequestFrame(); // recovery only; the open overlay owns steady sampling
        }
    }
    void RegisterMenu() {
        if (registered) return;
        menuName = BSFixedString("TrueHUD"); hudName = BSFixedString("HUD Menu");
        auto manager = MenuManager::GetSingleton();
        if (manager) { manager->Register("TrueHUD", CreateMenu); registered = true; }
    }
    void RequestMenuOpen() {
        if (uiTasks && registered && InterlockedCompareExchange(&openPending, 1, 0) == 0)
            uiTasks->AddUITask(&openTask);
    }
}
