// Compile and exercise the real menu implementation, not a duplicate policy.
#include "../src/UI.cpp"
#include <intrin.h>
namespace TrueHUDLE {
    namespace {
        void* uiMovieTable[80]{};
        struct RawUIMovie { void** table; UInt32 references; } uiMovie{uiMovieTable, 1};
        unsigned uiAdvances = 0, uiRenders = 0, uiDestructions = 0;
        unsigned uiInputEvents = 0;
        struct ObjectProbe {
            unsigned configs = 0, positions = 0, updates = 0, progress = 0;
            double healthFraction = 0;
            bool failConfig = false, failPosition = false;
        } uiBoss, uiShout;
        GFxValue::ObjectInterface uiObjectInterface{};
        unsigned uiDepthSorts = 0, uiDepthWrites = 0;
        unsigned uiCustomSorts = 0;
        void* uiCustomArray = nullptr;
        bool uiFailCustomSort = false, uiFailCustomRead = false;
        bool uiFailSort = false, uiFailMember = false;
        unsigned uiRowIndex = 0;
        unsigned char uiRows[512]{};
        void TestObject(GFxValue* value, void* object, UInt32 type = GFxValue::kType_DisplayObject) {
            value->SetUndefined(); value->objectInterface = &uiObjectInterface;
            value->type = type; value->data.obj = object;
        }
        bool __fastcall TestObjectInvoke(void*, void*, void* object, GFxValue* result,
                                        const char* name, GFxValue* args, UInt32 count, bool) {
            auto& probe = *static_cast<ObjectProbe*>(object);
            if (!strcmp(name, "loadConfig")) { ++probe.configs; return !probe.failConfig; }
            if (!strcmp(name, "setProgress")) { ++probe.progress; return true; }
            if (!strcmp(name, "updateData")) { ++probe.updates; if (count == 18) probe.healthFraction = args[11].GetNumber(); }
            if (!strcmp(name, "isReadyToRemove") && result) result->SetBool(false);
            return true;
        }
        bool __fastcall TestSetDisplay(void*, void*, void* object, GFxValue::DisplayInfo*) {
            auto& probe = *static_cast<ObjectProbe*>(object); ++probe.positions; return !probe.failPosition;
        }
        bool __fastcall TestSetMember(void*, void*, void*, const char*, GFxValue*, bool) {
            ++uiDepthWrites; return !uiFailMember;
        }
        bool __fastcall TestPushBack(void*, void*, void*, GFxValue*) { return true; }
        void __fastcall TestCreateObject(void*, void*, GFxValue* value, const char*, GFxValue*, UInt32) {
            TestObject(value, &uiRows[(uiRowIndex++) % sizeof(uiRows)], GFxValue::kType_Object);
        }
        void __fastcall TestCreateArray(void*, void*, GFxValue* value) {
            TestObject(value, &uiRows[(uiRowIndex++) % sizeof(uiRows)], GFxValue::kType_Array);
        }
        bool testCustomOpen = true;
        unsigned testOpenPosts = 0, testOpenMessages = 0;
        UIDelegate_v1* testOpenTask = nullptr;
        void __fastcall TestUnexpected(void*, void*) {
            printf("  FAIL  unexpected fake movie virtual slot caller=%p\n", _ReturnAddress());
            RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, nullptr);
        }
        alignas(16) unsigned char uiMenuStorage[sizeof(TrueHUDMenu)];
        alignas(4) unsigned char uiManagerBytes[0x11C]{};
        bool __fastcall TestLoadMovie(void*, void*, IMenu*, GFxMovieView** output, const char*, UInt32, float) {
            *output = reinterpret_cast<GFxMovieView*>(&uiMovie); return true;
        }
        void __fastcall TestAdvance(IMenu*, void*, UInt32, UInt32) { ++uiAdvances; }
        void __fastcall TestMenuDestruct(IMenu*, void*) { ++uiDestructions; }
        bool __fastcall TestMenuOpen(MenuManager*, void*, BSFixedString* name) {
            return name != &menuName || testCustomOpen;
        }
        void __cdecl TestQueueUI(UIDelegate_v1* task) {
            Check(!testOpenTask, "at most one menu-open UI delegate pending");
            testOpenTask = task; ++testOpenPosts;
        }
        void __fastcall TestOpenMessage(UIManager*, void*, BSFixedString* name, UInt32 type, void* data) {
            Check(name == &menuName && type == 1 && !data, "native HUD recovery posts the correct show message");
            testCustomOpen = true; ++testOpenMessages;
        }
        void TestNativeTick() {
            using Call = void (__thiscall*)(IMenu*, UInt32, UInt32);
            (*reinterpret_cast<Call*>(0x010E5390))(reinterpret_cast<IMenu*>(hudBytes), 0x3C888889, 0x55667788);
        }
        void DrainOpenTask() {
            if (!testOpenTask) return;
            auto task = testOpenTask; testOpenTask = nullptr;
            task->Run(); task->Dispose();
        }
        void __fastcall TestSetVariable(void*, void*, const char* name, GFxValue* value, UInt32) {
            if (!strcmp(name, "_root.__TrueHUDCustomDepth")) uiCustomArray = value->data.obj;
        }
        bool __fastcall TestGetVariable(void*, void*, GFxValue* value, const char* name) {
            if (!strcmp(name, "_root.__TrueHUDCustomDepth") && uiCustomArray && !uiFailCustomRead) {
                TestObject(value, uiCustomArray, GFxValue::kType_Array); return true;
            }
            value->SetUndefined(); return false;
        }
        bool __fastcall TestInvoke(void*, void*, const char* name, GFxValue* result, GFxValue*, UInt32) {
            if (!strcmp(name, "_root.TrueHUD.AddShoutIndicatorWidget") && result) TestObject(result, &uiShout);
            if (!strcmp(name, "_root.TrueHUD.SortDepths")) { ++uiDepthSorts; return !uiFailSort; }
            if (!strcmp(name, "_root.TrueHUD.SortCustomWidgetDepths")) { ++uiCustomSorts; return !uiFailCustomSort; }
            return true;
        }
        void __fastcall TestRender(void*, void*) { ++uiRenders; }
        UInt32 __fastcall TestHandleEvent(void*, void*, void*) { ++uiInputEvents; return 0; }
        // Same hidden-return-pointer ABI as LoadMovie's native +7C call.
        __declspec(naked) void TestVisibleRect() {
            __asm {
                mov eax, [esp+4]
                mov dword ptr [eax], 0
                mov dword ptr [eax+4], 0
                mov dword ptr [eax+8], 044A00000h
                mov dword ptr [eax+12], 044340000h
                ret 4
            }
        }
        struct UIRegressionAccess {
            static void SetupBoss(TrueHUDMenu& menu) {
                menu.settingsKnown = true; menu.frame.count = 1;
                auto& sample = menu.frame.actors[0]; sample = ActorSample{}; sample.handle = 7; sample.boss = true;
                sample.hostile = sample.combat = true; sample.level = 20;
                sample.maximum[0] = 200; sample.maximum[1] = sample.maximum[2] = 100;
                strcpy_s(sample.name, "Boss fixture");
                auto& widget = menu.widgets[0]; widget.handle = 7; widget.boss = true;
                TestObject(&widget.object, &uiBoss);
            }
            static void BossFrame(TrueHUDMenu& menu, Settings& settings, float health, float opacity = .8f) {
                menu.frame.actors[0].current[0] = health;
                GRectF rect{}; rect.right = 1280; rect.bottom = 720;
                menu.Actors(settings, rect, opacity); menu.ModifyHUD(settings, rect, opacity); menu.SortDepths();
            }
            static void ShoutFrame(TrueHUDMenu& menu, Settings& settings, unsigned bosses, float opacity) {
                menu.visibleBosses = bosses;
                GRectF rect{}; rect.right = 1280; rect.bottom = 720;
                menu.ModifyHUD(settings, rect, opacity);
            }
            static void Depth(TrueHUDMenu& menu, float depth) { menu.widgets[0].depth = depth; menu.SortDepths(); }
        };
        struct SortProbeWidget : TRUEHUD_API::WidgetBase {
            void Update(float) override {} void Initialize() override {} void Dispose() override {}
        };
        struct SortProbeRegistry {
            SortProbeWidget widget;
            template<class Visitor> void Visit(Visitor visitor) { visitor(4, 1, 2, widget); }
        };
    }
    void TestRealUILifecycle() {
        // Exercise the production conversion used by player, mount, actor/boss
        // and special-resource updateData/updateSpecial arguments. AS2 multiplies
        // this fraction by 100 itself to obtain the mask scale.
        const float remaining[] = {200.f, 150.f, 100.f, 50.f, 2.f, 1.f, 0.f, 80.f, 200.f};
        const float fractions[] = {1.f, .75f, .5f, .25f, .01f, .005f, 0.f, .4f, 1.f};
        for (unsigned i = 0; i < sizeof(remaining)/sizeof(*remaining); ++i)
            Check(std::fabs(Percent(remaining[i], 200.f) - fractions[i]) < .000001f,
                "live resource depletion and regeneration reach AS2 as unit fractions, not 0-100 values");
        Check(Percent(250.f, 200.f) == 1.f && Percent(-20.f, 200.f) == 0.f,
            "AS2 fill fraction clamps over-cap and negative resources");
        Check(Percent(50.f, 0.f) == 0.f && Percent(50.f, -1.f) == 0.f,
            "AS2 fill fraction handles absent resource capacity");
        const auto oldManager = *reinterpret_cast<MenuManager**>(0x012E3548);
        const auto oldLoader = *reinterpret_cast<GFxLoader**>(0x01B2E9B0);
        const auto oldNames = *reinterpret_cast<void**>(0x012E35E0);
        const auto oldHUDName = hudName.data;
        const auto oldMenuName = menuName.data;
        const auto oldUI = *reinterpret_cast<UIManager**>(0x012E35E4);
        auto oldTasks = uiTasks;
        const bool oldRegistered = registered;
        const int previousFaults = faults;
        faults = 0;
        FakeGame::WriteStub(0x00A60C80, reinterpret_cast<const void*>(&TestLoadMovie));
        FakeGame::WriteStub(0x00A64980, reinterpret_cast<const void*>(&TestAdvance));
        FakeGame::WriteStub(0x00A64A10, reinterpret_cast<const void*>(&TestMenuDestruct));
        FakeGame::WriteStub(0x00A5CE90, reinterpret_cast<const void*>(&TestMenuOpen));
        FakeGame::WriteStub(0x00431B00, reinterpret_cast<const void*>(&TestOpenMessage));
        // Existing SKSE GFxValue wrapper contracts, not new production hooks.
        FakeGame::WriteStub(0x00922ED0, reinterpret_cast<const void*>(&TestObjectInvoke));
        FakeGame::WriteStub(0x009211B0, reinterpret_cast<const void*>(&TestSetDisplay));
        FakeGame::WriteStub(0x00920D20, reinterpret_cast<const void*>(&TestSetMember));
        FakeGame::WriteStub(0x00920EF0, reinterpret_cast<const void*>(&TestPushBack));
        for (auto& slot : uiMovieTable) slot = reinterpret_cast<void*>(&TestUnexpected);
        uiMovieTable[16] = reinterpret_cast<void*>(&TestSetVariable);
        uiMovieTable[17] = reinterpret_cast<void*>(&TestGetVariable);
        uiMovieTable[13] = reinterpret_cast<void*>(&TestCreateObject);
        uiMovieTable[14] = reinterpret_cast<void*>(&TestCreateArray);
        // MSVC reverses these overloaded virtual declarations: array Invoke is
        // +5C, formatted Invoke is +58. Do not count SDK declarations naively.
        uiMovieTable[23] = reinterpret_cast<void*>(&TestInvoke);
        uiMovieTable[31] = reinterpret_cast<void*>(&TestVisibleRect);
        uiMovieTable[38] = reinterpret_cast<void*>(&TestRender);
        uiMovieTable[45] = reinterpret_cast<void*>(&TestHandleEvent);
        *reinterpret_cast<MenuManager**>(0x012E3548) = reinterpret_cast<MenuManager*>(uiManagerBytes);
        uiManagerBytes[0x118] = 1;
        *reinterpret_cast<GFxLoader**>(0x01B2E9B0) = reinterpret_cast<GFxLoader*>(&uiMovie);
        *reinterpret_cast<void**>(0x012E35E0) = nullptr;
        hudName.data = nullptr;
        ResetState(false); Drain();
        auto menu = new (uiMenuStorage) TrueHUDMenu;
        IMenu* engineMenu = menu;
        const float interval = 1.f/60.f; UInt32 word; memcpy(&word, &interval, 4);
        for (unsigned i = 0; i < 180; ++i) {
            engineMenu->NextFrame(word, i); engineMenu->Render(); Drain();
        }
        Check(uiAdvances == 180 && uiRenders == 180 && !faults,
            "real UI menu repeatedly advances/renders while player snapshot is not ready");
        // Actual SDK message/data types, obtained through the public signature.
        // Native routing continues to the underlying menu only for result2.
        std::remove_pointer_t<MenuMessagePointer> inputMessage{};
        std::remove_pointer_t<decltype(inputMessage.data)> inputData{};
        inputMessage.unk04 = 6; inputMessage.data = &inputData;
        unsigned underlyingInputs = 0, closeRequests = 0;
        for (unsigned payload = 0; payload < 4; ++payload) {
            inputData.unk08 = &payload; // opaque payloads: routing must not inspect them
            const UInt32 result = engineMenu->ProcessUnkData1(&inputMessage);
            if (result == 2) { ++underlyingInputs; if (payload != 2) ++closeRequests; }
            Check(result == 2 && uiInputEvents == 0,
                "hidden HUD overlay leaves keyboard/mouse/gamepad events to the underlying menu");
        }
        Check(underlyingInputs == 4 && closeRequests == 3,
            "underlying menu receives navigation and Tab/Escape/back close requests");
        inputMessage.data = nullptr;
        Check(engineMenu->ProcessUnkData1(&inputMessage) == 2,
            "overlay declines an input message with missing event data");
        bool lifecycleUnchanged = true;
        for (UInt32 kind : {0u, 1u, 3u, 7u}) {
            inputMessage.unk04 = kind;
            lifecycleUnchanged = engineMenu->ProcessUnkData1(&inputMessage) == 2 && lifecycleUnchanged;
        }
        Check(lifecycleUnchanged, "overlay preserves default show/hide/noninput routing results");
        SKSETaskInterface testTasks{}; testTasks.AddUITask = TestQueueUI;
        InitializeUI(&testTasks); registered = true;
        menuName.data = "TrueHUD";
        *reinterpret_cast<UIManager**>(0x012E35E4) = reinterpret_cast<UIManager*>(&uiMovie);
        testCustomOpen = false;
        for (unsigned i = 0; i < 10; ++i) TestNativeTick();
        Check(testOpenPosts == 1 && testOpenTask,
            "native HUD frames recover a closed TrueHUD menu with one pending UI job");
        DrainOpenTask(); Drain();
        bool redundantSampling = false;
        for (unsigned i = 0; i < 120; ++i) {
            TestNativeTick(); redundantSampling |= scheduled != nullptr; Drain();
        }
        Check(!redundantSampling, "open overlay owns steady sampling without a second native HUD request");
        Check(testOpenPosts == 1 && testOpenMessages == 1 && testCustomOpen,
            "open menu requires no steady-frame UI posts");
        testCustomOpen = false; TestNativeTick(); DrainOpenTask(); Drain();
        Check(testOpenPosts == 2 && testOpenMessages == 2 && testCustomOpen,
            "native HUD frame recovers again after a later load-style menu closure");
        // Actual Actors/ModifyHUD/SortDepths with a live fake boss/shout clip.
        *reinterpret_cast<MenuManager**>(0x012E3548) = nullptr;
        Settings bossSettings; bossSettings.bBossBarUseHUDOpacity = 1;
        UIRegressionAccess::SetupBoss(*menu);
        for (unsigned i = 0; i < 120; ++i) UIRegressionAccess::BossFrame(*menu, bossSettings, 200.f-i);
        printf("boss UI fixture:120 frames, shout-config=%u boss-config=%u positions=%u sorts=%u live-updates=%u\n",
            uiShout.configs, uiBoss.configs, uiShout.positions+uiBoss.positions, uiDepthSorts, uiBoss.updates);
        Check(uiShout.configs == 1 && uiBoss.configs == 1, "boss display does not rebuild shout timelines each frame");
        Check(uiShout.positions == 1 && uiBoss.positions == 1, "unchanged boss and shout transforms are not resent");
        Check(uiDepthSorts == 1, "unchanged fixed boss depths do not sort and swap every frame");
        Check(uiBoss.updates == 120 && std::fabs(uiBoss.healthFraction-.405) < .00001,
            "boss optimization preserves every live resource update");
        Check(uiShout.progress == 120, "shout timeline progress and fade keepalive remain live");
        APICommand color; color.kind = APICommandKind::OverrideColor; color.actor = 99; color.value = 0xAABBCC;
        ProcessAPIColor(color); UIRegressionAccess::BossFrame(*menu, bossSettings, 80);
        Check(uiBoss.configs == 1, "an unrelated actor color change does not rebuild boss timelines");
        color.actor = 7; ProcessAPIColor(color); UIRegressionAccess::BossFrame(*menu, bossSettings, 79);
        Check(uiBoss.configs == 2, "boss-specific color changes reconfigure the affected boss immediately");
        color.kind = APICommandKind::RevertColor; ProcessAPIColor(color); UIRegressionAccess::BossFrame(*menu, bossSettings, 78);
        Check(uiBoss.configs == 3, "reverting the last color restores the boss defaults");
        UIRegressionAccess::ShoutFrame(*menu, bossSettings, 1, .6f);
        Check(uiShout.configs == 2, "changed shout opacity reconfigures once");
        UIRegressionAccess::ShoutFrame(*menu, bossSettings, 0, .6f);
        UIRegressionAccess::ShoutFrame(*menu, bossSettings, 1, .6f);
        Check(uiShout.configs == 3, "recreated shout clip receives its configuration again");
        uiShout.failConfig = true;
        UIRegressionAccess::ShoutFrame(*menu, bossSettings, 1, .5f);
        uiShout.failConfig = false;
        UIRegressionAccess::ShoutFrame(*menu, bossSettings, 1, .5f);
        Check(uiShout.configs == 5, "failed shout configuration retries instead of caching a failed call");
        UIRegressionAccess::Depth(*menu, .5f); const auto movedSorts = uiDepthSorts;
        UIRegressionAccess::Depth(*menu, .5f);
        Check(movedSorts > 1 && uiDepthSorts == movedSorts, "changed depth sorts immediately and stable depth stays quiet");
        uiFailSort = true; UIRegressionAccess::Depth(*menu, .6f); const auto failedSorts = uiDepthSorts;
        uiFailSort = false; UIRegressionAccess::Depth(*menu, .6f);
        Check(uiDepthSorts == failedSorts+1, "failed depth sort is retried without another depth change");
        DisplayMemo position;
        GFxValue positionObject; TestObject(&positionObject, &uiBoss);
        const auto positionStart = uiBoss.positions;
        uiBoss.failPosition = true; Position(positionObject, 10, 20, 100, true, &position);
        uiBoss.failPosition = false; Position(positionObject, 10, 20, 100, true, &position);
        Position(positionObject, 10, 20, 100, true, &position);
        Check(uiBoss.positions == positionStart+2, "failed display transform retries; successful unchanged transform is skipped");
        Position(positionObject, 11, 20, 100, true, &position);
        Position(positionObject, 11, 20, 100, false, &position);
        Check(uiBoss.positions == positionStart+4, "position and visibility changes are applied immediately");
        GFxMovieView* customView = reinterpret_cast<GFxMovieView*>(&uiMovie);
        CustomHost customHost{customView}; SortProbeRegistry customRegistry;
        customHost.rows[0].used = true; customHost.rows[0].owner = 4; customHost.rows[0].type = 1; customHost.rows[0].id = 2;
        for (unsigned i = 0; i < 120; ++i) customHost.Sort(customRegistry);
        Check(uiCustomSorts == 1, "unchanged custom-widget depth does not resort its owner every frame");
        customRegistry.widget._depth = .5f; customHost.Sort(customRegistry);
        Check(uiCustomSorts == 2, "changed custom-widget depth sorts its owner immediately");
        uiFailCustomSort = true; customRegistry.widget._depth = .6f; customHost.Sort(customRegistry);
        uiFailCustomSort = false; customHost.Sort(customRegistry);
        Check(uiCustomSorts == 4, "failed custom-widget sort retries with unchanged depth");
        uiFailCustomRead = true; customRegistry.widget._depth = .7f; customHost.Sort(customRegistry);
        uiFailCustomRead = false; customHost.Sort(customRegistry);
        Check(uiCustomSorts == 5, "failed custom array lookup retries with unchanged depth");
        Timing disabledTiming{"fixture"};
        Check(disabledTiming.Begin() == 0, "disabled logging also skips timing queries");
        registered = oldRegistered; uiTasks = oldTasks;
        menuName.data = oldMenuName;
        *reinterpret_cast<UIManager**>(0x012E35E4) = oldUI;
        *reinterpret_cast<MenuManager**>(0x012E3548) = reinterpret_cast<MenuManager*>(4);
        hudName.data = "HUD Menu";
        menu->~TrueHUDMenu();
        Check(uiDestructions == 1 && !faults,
            "real UI destructor clears owned clips without consulting a destroyed foreign menu manager");
        *reinterpret_cast<MenuManager**>(0x012E3548) = oldManager;
        *reinterpret_cast<GFxLoader**>(0x01B2E9B0) = oldLoader;
        *reinterpret_cast<void**>(0x012E35E0) = oldNames;
        hudName.data = oldHUDName;
        faults = previousFaults;
        ResetState(true);
    }
}
