; TrueHUD LE adaptation of Ersh TrueHUD 1.1.10. GPL-3.0-or-later WITH EXCEPTIONS.

ScriptName TrueHUD_MCM extends SKI_ConfigBase

Bool Property bBossBarsHideCompass Auto

String[] _ids

String[] _help

Int[] _types

Float[] _mins

Float[] _maxs

Float[] _steps

Float[] _defaults

Float Function ReadSetting(String id) native

Function WriteSetting(String id, Float value) native

Function CommitSettings() native

Event OnConfigInit()

    ModName = "TrueHUD"

    Pages = new String[6]

    Pages[0] = "$TrueHUD_GeneralPage"

    Pages[1] = "$TrueHUD_ActorInfoBars"

    Pages[2] = "$TrueHUD_BossBars"

    Pages[3] = "$TrueHUD_PlayerWidget"

    Pages[4] = "$TrueHUD_RecentLoot"

    Pages[5] = "$TrueHUD_Colors"

EndEvent

Int Function GetVersion()

    Return 1

EndFunction

Event OnConfigClose()

    CommitSettings()

EndEvent

Event OnPageReset(String page)

    _ids = new String[128]

    _help = new String[128]

    _types = new Int[128]

    _mins = new Float[128]

    _maxs = new Float[128]

    _steps = new Float[128]

    _defaults = new Float[128]

    SetCursorFillMode(TOP_TO_BOTTOM)

    SetCursorPosition(0)

    If page == Pages[0] || page == ""

        Page0()

    ElseIf page == Pages[1]

        Page1()

    ElseIf page == Pages[2]

        Page2()

    ElseIf page == Pages[3]

        Page3()

    ElseIf page == Pages[4]

        Page4()

    ElseIf page == Pages[5]

        Page5()

    EndIf

EndEvent

Function Page0()

    Int option

    Int index

    Float value

    AddHeaderOption("$TrueHUD_GeneralSettings_HeaderText", 0)

    option = AddToggleOption("$TrueHUD_EnableActorInfoBars_OptionText", ReadSetting("bEnableActorInfoBars:General") != 0, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "bEnableActorInfoBars:General"

        _types[index] = 1

        _help[index] = "$TrueHUD_EnableActorInfoBars_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_EnableBossBars_OptionText", ReadSetting("bEnableBossBars:General") != 0, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "bEnableBossBars:General"

        _types[index] = 1

        _help[index] = "$TrueHUD_EnableBossBars_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_EnablePlayerWidget_OptionText", ReadSetting("bEnablePlayerWidget:General") != 0, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "bEnablePlayerWidget:General"

        _types[index] = 1

        _help[index] = "$TrueHUD_EnablePlayerWidget_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_EnableRecentLoot_OptionText", ReadSetting("bEnableRecentLoot:General") != 0, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "bEnableRecentLoot:General"

        _types[index] = 1

        _help[index] = "$TrueHUD_EnableRecentLoot_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_HideVanillaTargetBar_OptionText", ReadSetting("bHideVanillaTargetBar:General") != 0, FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bHideVanillaTargetBar:General"

        _types[index] = 1

        _help[index] = "$TrueHUD_HideVanillaTargetBar_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

EndFunction

Function Page1()

    Int option

    Int index

    Float value

    AddHeaderOption("$TrueHUD_DisplayCriteria_HeaderText", 0)

    option = AddMenuOption("$TrueHUD_InfoBarDisplayHostiles_OptionText", Label("uInfoBarDisplayHostiles:ActorInfoBars", ReadSetting("uInfoBarDisplayHostiles:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarDisplayHostiles:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarDisplayHostiles_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarDisplayTeammates_OptionText", Label("uInfoBarDisplayTeammates:ActorInfoBars", ReadSetting("uInfoBarDisplayTeammates:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarDisplayTeammates:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarDisplayTeammates_InfoText"

        _defaults[index] = 2.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarDisplayOthers_OptionText", Label("uInfoBarDisplayOthers:ActorInfoBars", ReadSetting("uInfoBarDisplayOthers:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarDisplayOthers:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarDisplayOthers_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("", 0)

    option = AddToggleOption("$TrueHUD_InfoBarDisplayPhantomBars_OptionText", ReadSetting("bInfoBarDisplayPhantomBars:ActorInfoBars") != 0, FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bInfoBarDisplayPhantomBars:ActorInfoBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_InfoBarDisplayPhantomBars_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarDisplayName_OptionText", Label("uInfoBarDisplayName:ActorInfoBars", ReadSetting("uInfoBarDisplayName:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarDisplayName:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarDisplayName_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarDisplayDamageCounter_OptionText", Label("uInfoBarDisplayDamageCounter:ActorInfoBars", ReadSetting("uInfoBarDisplayDamageCounter:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarDisplayDamageCounter:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarDisplayDamageCounter_InfoText"

        _defaults[index] = 4.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarDisplayIndicator_OptionText", Label("uInfoBarDisplayIndicator:ActorInfoBars", ReadSetting("uInfoBarDisplayIndicator:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarDisplayIndicator:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarDisplayIndicator_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("", 0)

    option = AddMenuOption("$TrueHUD_InfoBarResourcesModeTarget_OptionText", Label("uInfoBarResourcesModeTarget:ActorInfoBars", ReadSetting("uInfoBarResourcesModeTarget:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarResourcesModeTarget:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarResourcesModeTarget_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarResourcesModeHostiles_OptionText", Label("uInfoBarResourcesModeHostiles:ActorInfoBars", ReadSetting("uInfoBarResourcesModeHostiles:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarResourcesModeHostiles:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarResourcesModeHostiles_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarResourcesModeTeammates_OptionText", Label("uInfoBarResourcesModeTeammates:ActorInfoBars", ReadSetting("uInfoBarResourcesModeTeammates:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarResourcesModeTeammates:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarResourcesModeTeammates_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarResourcesModeOthers_OptionText", Label("uInfoBarResourcesModeOthers:ActorInfoBars", ReadSetting("uInfoBarResourcesModeOthers:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarResourcesModeOthers:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarResourcesModeOthers_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_SpecialBar_HeaderText", 0)

    If ((ReadSetting("bEnableActorInfoBars:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))

        AddTextOption("$TrueHUD_SpecialBars_OptionText", "$TrueHUD_SpecialBarsControlled_ValueText", 1)

    EndIf

    If ((ReadSetting("bEnableActorInfoBars:General") != 0) && !((ReadSetting("__SpecialResourcesControlled") != 0)))

        AddTextOption("$TrueHUD_SpecialBars_OptionText", "$TrueHUD_SpecialBarsNotControlled_ValueText", 1)

    EndIf

    option = AddToggleOption("$TrueHUD_InfoBarDisplaySpecialPhantomBar_OptionText", ReadSetting("bInfoBarDisplaySpecialPhantomBar:ActorInfoBars") != 0, FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "bInfoBarDisplaySpecialPhantomBar:ActorInfoBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_InfoBarDisplaySpecialPhantomBar_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarSpecialModeTarget_OptionText", Label("uInfoBarSpecialModeTarget:ActorInfoBars", ReadSetting("uInfoBarSpecialModeTarget:ActorInfoBars") as Int), FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarSpecialModeTarget:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarSpecialModeTarget_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarSpecialModeHostiles_OptionText", Label("uInfoBarSpecialModeHostiles:ActorInfoBars", ReadSetting("uInfoBarSpecialModeHostiles:ActorInfoBars") as Int), FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarSpecialModeHostiles:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarSpecialModeHostiles_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarSpecialModeTeammates_OptionText", Label("uInfoBarSpecialModeTeammates:ActorInfoBars", ReadSetting("uInfoBarSpecialModeTeammates:ActorInfoBars") as Int), FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarSpecialModeTeammates:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarSpecialModeTeammates_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarSpecialModeOthers_OptionText", Label("uInfoBarSpecialModeOthers:ActorInfoBars", ReadSetting("uInfoBarSpecialModeOthers:ActorInfoBars") as Int), FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarSpecialModeOthers:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarSpecialModeOthers_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    SetCursorPosition(1)

    AddHeaderOption("$TrueHUD_AlignmentSettings_HeaderText", 0)

    option = AddMenuOption("$TrueHUD_InfoBarDirection_OptionText", Label("uInfoBarDirection:ActorInfoBars", ReadSetting("uInfoBarDirection:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarDirection:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarDirection_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarDamageCounterAlignment_OptionText", Label("uInfoBarDamageCounterAlignment:ActorInfoBars", ReadSetting("uInfoBarDamageCounterAlignment:ActorInfoBars") as Int), FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && (ReadSetting("uInfoBarDisplayDamageCounter:ActorInfoBars") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarDamageCounterAlignment:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarDamageCounterAlignment_InfoText"

        _defaults[index] = 2.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_MiscSettings_HeaderText", 0)

    option = AddToggleOption("$TrueHUD_InfoBarLevelColorOutline_OptionText", ReadSetting("bInfoBarLevelColorOutline:ActorInfoBars") != 0, FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bInfoBarLevelColorOutline:ActorInfoBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_InfoBarLevelColorOutline_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_InfoBarLevelThreshold_OptionText", ReadSetting("uInfoBarLevelThreshold:ActorInfoBars"), "{2}", FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarLevelThreshold:ActorInfoBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_InfoBarLevelThreshold_InfoText"

        _defaults[index] = 10.0

        _mins[index] = 0

        _maxs[index] = 50

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_InfoBarIndicatorMode_OptionText", Label("uInfoBarIndicatorMode:ActorInfoBars", ReadSetting("uInfoBarIndicatorMode:ActorInfoBars") as Int), FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && (ReadSetting("uInfoBarDisplayIndicator:ActorInfoBars") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarIndicatorMode:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarIndicatorMode_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_InfoBarPhantomDuration_OptionText", ReadSetting("fInfoBarPhantomDuration:ActorInfoBars"), "{2}", FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && (ReadSetting("bInfoBarDisplayPhantomBars:ActorInfoBars") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fInfoBarPhantomDuration:ActorInfoBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_InfoBarPhantomDuration_InfoText"

        _defaults[index] = 0.75

        _mins[index] = 0.01

        _maxs[index] = 5

        _steps[index] = 0.01

    EndIf

    option = AddSliderOption("$TrueHUD_InfoBarDamageCounterDuration_OptionText", ReadSetting("fInfoBarDamageCounterDuration:ActorInfoBars"), "{2}", FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && (ReadSetting("uInfoBarDisplayDamageCounter:ActorInfoBars") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fInfoBarDamageCounterDuration:ActorInfoBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_InfoBarDamageCounterDuration_InfoText"

        _defaults[index] = 2.0

        _mins[index] = 0.01

        _maxs[index] = 5

        _steps[index] = 0.01

    EndIf

    AddHeaderOption("$TrueHUD_DisplaySettings_HeaderText", 0)

    option = AddMenuOption("$TrueHUD_InfoBarAnchor_OptionText", Label("uInfoBarAnchor:ActorInfoBars", ReadSetting("uInfoBarAnchor:ActorInfoBars") as Int), FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uInfoBarAnchor:ActorInfoBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_InfoBarAnchor_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_InfoBarOffsetZ_OptionText", ReadSetting("fInfoBarOffsetZ:ActorInfoBars"), "{2}", FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fInfoBarOffsetZ:ActorInfoBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_InfoBarOffsetZ_InfoText"

        _defaults[index] = 30.0

        _mins[index] = -200

        _maxs[index] = 200

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_InfoBarScale_OptionText", ReadSetting("fInfoBarScale:ActorInfoBars"), "{2}", FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fInfoBarScale:ActorInfoBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_InfoBarScale_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.1

        _maxs[index] = 2

        _steps[index] = 0.1

    EndIf

    option = AddToggleOption("$TrueHUD_InfoBarScaleWithDistance_OptionText", ReadSetting("bInfoBarScaleWithDistance:ActorInfoBars") != 0, FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bInfoBarScaleWithDistance:ActorInfoBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_InfoBarScaleWithDistance_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_InfoBarUseHUDOpacity_OptionText", ReadSetting("bInfoBarUseHUDOpacity:ActorInfoBars") != 0, FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bInfoBarUseHUDOpacity:ActorInfoBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_InfoBarUseHUDOpacity_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_InfoBarOpacity_OptionText", ReadSetting("fInfoBarOpacity:ActorInfoBars"), "{2}", FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && !((ReadSetting("bInfoBarUseHUDOpacity:ActorInfoBars") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fInfoBarOpacity:ActorInfoBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_InfoBarOpacity_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.01

        _maxs[index] = 1

        _steps[index] = 0.01

    EndIf

    option = AddSliderOption("$TrueHUD_InfoBarWidth_OptionText", ReadSetting("fInfoBarWidth:ActorInfoBars"), "{2}", FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fInfoBarWidth:ActorInfoBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_InfoBarWidth_InfoText"

        _defaults[index] = 60.0

        _mins[index] = 30

        _maxs[index] = 300

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_InfoBarResourceWidth_OptionText", ReadSetting("fInfoBarResourceWidth:ActorInfoBars"), "{2}", FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && ((ReadSetting("uInfoBarResourcesModeTarget:ActorInfoBars") != 0) || (ReadSetting("uInfoBarResourcesModeHostiles:ActorInfoBars") != 0) || (ReadSetting("uInfoBarResourcesModeTeammates:ActorInfoBars") != 0) || (ReadSetting("uInfoBarResourcesModeOthers:ActorInfoBars") != 0) || ((ReadSetting("__SpecialResourcesControlled") != 0) && ((ReadSetting("uInfoBarSpecialModeTarget:ActorInfoBars") != 0) || (ReadSetting("uInfoBarSpecialModeHostiles:ActorInfoBars") != 0) || (ReadSetting("uInfoBarSpecialModeTeammates:ActorInfoBars") != 0) || (ReadSetting("uInfoBarSpecialModeOthers:ActorInfoBars") != 0)))))))

    If option >= 0

        index = option % 256

        _ids[index] = "fInfoBarResourceWidth:ActorInfoBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_InfoBarResourceWidth_InfoText"

        _defaults[index] = 30.0

        _mins[index] = 30

        _maxs[index] = 300

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_InfoBarTargetWidth_OptionText", ReadSetting("fInfoBarTargetWidth:ActorInfoBars"), "{2}", FlagsFor((ReadSetting("bEnableActorInfoBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fInfoBarTargetWidth:ActorInfoBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_InfoBarTargetWidth_InfoText"

        _defaults[index] = 100.0

        _mins[index] = 30

        _maxs[index] = 300

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_InfoBarTargetResourceWidth_OptionText", ReadSetting("fInfoBarTargetResourceWidth:ActorInfoBars"), "{2}", FlagsFor(((ReadSetting("bEnableActorInfoBars:General") != 0) && ((ReadSetting("uInfoBarResourcesModeTarget:ActorInfoBars") != 0) || (ReadSetting("uInfoBarResourcesModeHostiles:ActorInfoBars") != 0) || (ReadSetting("uInfoBarResourcesModeTeammates:ActorInfoBars") != 0) || (ReadSetting("uInfoBarResourcesModeOthers:ActorInfoBars") != 0) || ((ReadSetting("__SpecialResourcesControlled") != 0) && ((ReadSetting("uInfoBarSpecialModeTarget:ActorInfoBars") != 0) || (ReadSetting("uInfoBarSpecialModeHostiles:ActorInfoBars") != 0) || (ReadSetting("uInfoBarSpecialModeTeammates:ActorInfoBars") != 0) || (ReadSetting("uInfoBarSpecialModeOthers:ActorInfoBars") != 0)))))))

    If option >= 0

        index = option % 256

        _ids[index] = "fInfoBarTargetResourceWidth:ActorInfoBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_InfoBarTargetResourceWidth_InfoText"

        _defaults[index] = 70.0

        _mins[index] = 30

        _maxs[index] = 300

        _steps[index] = 1

    EndIf

EndFunction

Function Page2()

    Int option

    Int index

    Float value

    AddHeaderOption("$TrueHUD_Presets_HeaderText", 0)

    option = AddTextOption("$TrueHUD_Preset_BossBarTop_OptionText", "$TrueHUD_LoadPreset_ValueText", FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "preset:BossBarTop"

        _types[index] = 5

        _help[index] = "$TrueHUD_Preset_BossBarTop_InfoText"

        _defaults[index] = 0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddTextOption("$TrueHUD_Preset_BossBarBottom_OptionText", "$TrueHUD_LoadPreset_ValueText", FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "preset:BossBarBottom"

        _types[index] = 5

        _help[index] = "$TrueHUD_Preset_BossBarBottom_InfoText"

        _defaults[index] = 0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_DisplayCriteria_HeaderText", 0)

    option = AddToggleOption("$TrueHUD_BossBarDisplayPhantomBars_OptionText", ReadSetting("bBossBarDisplayPhantomBars:BossBars") != 0, FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bBossBarDisplayPhantomBars:BossBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_BossBarDisplayPhantomBars_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_BossBarDisplayDamageCounter_OptionText", ReadSetting("bBossBarDisplayDamageCounter:BossBars") != 0, FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bBossBarDisplayDamageCounter:BossBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_BossBarDisplayDamageCounter_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_BossBarDisplayIndicator_OptionText", ReadSetting("bBossBarDisplayIndicator:BossBars") != 0, FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bBossBarDisplayIndicator:BossBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_BossBarDisplayIndicator_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_BossBarResourcesMode_OptionText", Label("uBossBarResourcesMode:BossBars", ReadSetting("uBossBarResourcesMode:BossBars") as Int), FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uBossBarResourcesMode:BossBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_BossBarResourcesMode_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_SpecialBar_HeaderText", 0)

    If ((ReadSetting("bEnableBossBars:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))

        AddTextOption("$TrueHUD_SpecialBars_OptionText", "$TrueHUD_SpecialBarsControlled_ValueText", 1)

    EndIf

    If ((ReadSetting("bEnableBossBars:General") != 0) && !((ReadSetting("__SpecialResourcesControlled") != 0)))

        AddTextOption("$TrueHUD_SpecialBars_OptionText", "$TrueHUD_SpecialBarsNotControlled_ValueText", 1)

    EndIf

    option = AddToggleOption("$TrueHUD_BossBarDisplaySpecialPhantomBar_OptionText", ReadSetting("bBossBarDisplaySpecialPhantomBar:BossBars") != 0, FlagsFor(((ReadSetting("bEnableBossBars:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "bBossBarDisplaySpecialPhantomBar:BossBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_BossBarDisplaySpecialPhantomBar_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_BossBarSpecialMode_OptionText", Label("uBossBarSpecialMode:BossBars", ReadSetting("uBossBarSpecialMode:BossBars") as Int), FlagsFor(((ReadSetting("bEnableBossBars:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uBossBarSpecialMode:BossBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_BossBarSpecialMode_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_AlignmentSettings_HeaderText", 0)

    option = AddMenuOption("$TrueHUD_BossBarDirection_OptionText", Label("uBossBarDirection:BossBars", ReadSetting("uBossBarDirection:BossBars") as Int), FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uBossBarDirection:BossBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_BossBarDirection_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_BossBarDamageCounterAlignment_OptionText", Label("uBossBarDamageCounterAlignment:BossBars", ReadSetting("uBossBarDamageCounterAlignment:BossBars") as Int), FlagsFor(((ReadSetting("bEnableBossBars:General") != 0) && (ReadSetting("bBossBarDisplayDamageCounter:BossBars") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uBossBarDamageCounterAlignment:BossBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_BossBarDamageCounterAlignment_InfoText"

        _defaults[index] = 2.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_BossBarResourceAlignment_OptionText", Label("uBossBarResourceAlignment:BossBars", ReadSetting("uBossBarResourceAlignment:BossBars") as Int), FlagsFor(((ReadSetting("bEnableBossBars:General") != 0) && ((ReadSetting("uBossBarResourcesMode:BossBars") != 0) || ((ReadSetting("__SpecialResourcesControlled") != 0) && (ReadSetting("uBossBarSpecialMode:BossBars") != 0))))))

    If option >= 0

        index = option % 256

        _ids[index] = "uBossBarResourceAlignment:BossBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_BossBarResourceAlignment_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_BossBarNameAlignment_OptionText", Label("uBossBarNameAlignment:BossBars", ReadSetting("uBossBarNameAlignment:BossBars") as Int), FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uBossBarNameAlignment:BossBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_BossBarNameAlignment_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    SetCursorPosition(1)

    AddHeaderOption("$TrueHUD_MiscSettings_HeaderText", 0)

    option = AddToggleOption("$TrueHUD_BossBarLevelColorOutline_OptionText", ReadSetting("bBossBarLevelColorOutline:BossBars") != 0, FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bBossBarLevelColorOutline:BossBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_BossBarLevelColorOutline_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_BossBarLevelThreshold_OptionText", ReadSetting("uBossBarLevelThreshold:BossBars"), "{2}", FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uBossBarLevelThreshold:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_BossBarLevelThreshold_InfoText"

        _defaults[index] = 10.0

        _mins[index] = 0

        _maxs[index] = 50

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_BossBarIndicatorMode_OptionText", Label("uBossBarIndicatorMode:BossBars", ReadSetting("uBossBarIndicatorMode:BossBars") as Int), FlagsFor(((ReadSetting("bEnableBossBars:General") != 0) && (ReadSetting("bBossBarDisplayIndicator:BossBars") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uBossBarIndicatorMode:BossBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_BossBarIndicatorMode_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_BossBarPhantomDuration_OptionText", ReadSetting("fBossBarPhantomDuration:BossBars"), "{2}", FlagsFor(((ReadSetting("bEnableBossBars:General") != 0) && (ReadSetting("bBossBarDisplayPhantomBars:BossBars") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fBossBarPhantomDuration:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_BossBarPhantomDuration_InfoText"

        _defaults[index] = 0.75

        _mins[index] = 0.01

        _maxs[index] = 5

        _steps[index] = 0.01

    EndIf

    option = AddSliderOption("$TrueHUD_BossBarDamageCounterDuration_OptionText", ReadSetting("fBossBarDamageCounterDuration:BossBars"), "{2}", FlagsFor(((ReadSetting("bEnableBossBars:General") != 0) && (ReadSetting("bBossBarDisplayDamageCounter:BossBars") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fBossBarDamageCounterDuration:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_BossBarDamageCounterDuration_InfoText"

        _defaults[index] = 2.0

        _mins[index] = 0.01

        _maxs[index] = 5

        _steps[index] = 0.01

    EndIf

    option = AddSliderOption("$TrueHUD_BossBarMaxCount_OptionText", ReadSetting("uBossBarMaxCount:BossBars"), "{2}", FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uBossBarMaxCount:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_BossBarMaxCount_InfoText"

        _defaults[index] = 3.0

        _mins[index] = 1

        _maxs[index] = 5

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_BossBarModifyHUD_OptionText", Label("uBossBarModifyHUD:BossBars", ReadSetting("uBossBarModifyHUD:BossBars") as Int), FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uBossBarModifyHUD:BossBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_BossBarModifyHUD_InfoText"

        _defaults[index] = 2.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_DisplayStandaloneShoutWidgetWhenHidingCompass_OptionText", ReadSetting("bDisplayStandaloneShoutWidgetWhenHidingCompass:BossBars") != 0, FlagsFor(((ReadSetting("bEnableBossBars:General") != 0) && (ReadSetting("uBossBarModifyHUD:BossBars") == 2))))

    If option >= 0

        index = option % 256

        _ids[index] = "bDisplayStandaloneShoutWidgetWhenHidingCompass:BossBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_DisplayStandaloneShoutWidgetWhenHidingCompass_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_DisplaySettings_HeaderText", 0)

    option = AddSliderOption("$TrueHUD_BossBarAnchorX_OptionText", ReadSetting("fBossBarAnchorX:BossBars"), "{2}", FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fBossBarAnchorX:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_BossBarAnchorX_InfoText"

        _defaults[index] = 0.5

        _mins[index] = -0.5

        _maxs[index] = 1.5

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_BossBarAnchorY_OptionText", ReadSetting("fBossBarAnchorY:BossBars"), "{2}", FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fBossBarAnchorY:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_BossBarAnchorY_InfoText"

        _defaults[index] = 0.08

        _mins[index] = 0.001

        _maxs[index] = 1

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_MultipleBossBarsOffset_OptionText", ReadSetting("fMultipleBossBarsOffset:BossBars"), "{2}", FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fMultipleBossBarsOffset:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_MultipleBossBarsOffset_InfoText"

        _defaults[index] = 50.0

        _mins[index] = 1

        _maxs[index] = 100

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_MultipleBossBarsStackDirection_OptionText", Label("uMultipleBossBarsStackDirection:BossBars", ReadSetting("uMultipleBossBarsStackDirection:BossBars") as Int), FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uMultipleBossBarsStackDirection:BossBars"

        _types[index] = 3

        _help[index] = "$TrueHUD_MultipleBossBarsStackDirection_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_BossBarScale_OptionText", ReadSetting("fBossBarScale:BossBars"), "{2}", FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fBossBarScale:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_BossBarScale_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.1

        _maxs[index] = 2

        _steps[index] = 0.1

    EndIf

    option = AddToggleOption("$TrueHUD_BossBarUseHUDOpacity_OptionText", ReadSetting("bBossBarUseHUDOpacity:BossBars") != 0, FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bBossBarUseHUDOpacity:BossBars"

        _types[index] = 1

        _help[index] = "$TrueHUD_BossBarUseHUDOpacity_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_BossBarOpacity_OptionText", ReadSetting("fBossBarOpacity:BossBars"), "{2}", FlagsFor(((ReadSetting("bEnableBossBars:General") != 0) && !((ReadSetting("bBossBarUseHUDOpacity:BossBars") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fBossBarOpacity:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_BossBarOpacity_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.01

        _maxs[index] = 1

        _steps[index] = 0.01

    EndIf

    option = AddSliderOption("$TrueHUD_BossBarWidth_OptionText", ReadSetting("fBossBarWidth:BossBars"), "{2}", FlagsFor((ReadSetting("bEnableBossBars:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fBossBarWidth:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_BossBarWidth_InfoText"

        _defaults[index] = 400.0

        _mins[index] = 100

        _maxs[index] = 1000

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_BossBarResourceWidth_OptionText", ReadSetting("fBossBarResourceWidth:BossBars"), "{2}", FlagsFor(((ReadSetting("bEnableBossBars:General") != 0) && ((ReadSetting("uBossBarResourcesMode:BossBars") != 0) || ((ReadSetting("__SpecialResourcesControlled") != 0) && (ReadSetting("uBossBarSpecialMode:BossBars") != 0))))))

    If option >= 0

        index = option % 256

        _ids[index] = "fBossBarResourceWidth:BossBars"

        _types[index] = 2

        _help[index] = "$TrueHUD_BossBarResourceWidth_InfoText"

        _defaults[index] = 100.0

        _mins[index] = 30

        _maxs[index] = 1000

        _steps[index] = 1

    EndIf

EndFunction

Function Page3()

    Int option

    Int index

    Float value

    AddHeaderOption("$TrueHUD_Presets_HeaderText", 0)

    option = AddTextOption("$TrueHUD_Preset_PlayerWidgetVanilla_OptionText", "$TrueHUD_LoadPreset_ValueText", FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "preset:PlayerWidgetVanilla"

        _types[index] = 5

        _help[index] = "$TrueHUD_Preset_PlayerWidgetVanilla_InfoText"

        _defaults[index] = 0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddTextOption("$TrueHUD_Preset_PlayerWidgetCombined_OptionText", "$TrueHUD_LoadPreset_ValueText", FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "preset:PlayerWidgetCombined"

        _types[index] = 5

        _help[index] = "$TrueHUD_Preset_PlayerWidgetCombined_InfoText"

        _defaults[index] = 0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_DisplayCriteria_HeaderText", 0)

    option = AddToggleOption("$TrueHUD_PlayerWidgetDisplayPhantomBars_OptionText", ReadSetting("bPlayerWidgetDisplayPhantomBars:PlayerWidget") != 0, FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bPlayerWidgetDisplayPhantomBars:PlayerWidget"

        _types[index] = 1

        _help[index] = "$TrueHUD_PlayerWidgetDisplayPhantomBars_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_PlayerWidgetHealthMode_OptionText", Label("uPlayerWidgetHealthMode:PlayerWidget", ReadSetting("uPlayerWidgetHealthMode:PlayerWidget") as Int), FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uPlayerWidgetHealthMode:PlayerWidget"

        _types[index] = 3

        _help[index] = "$TrueHUD_PlayerWidgetHealthMode_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_PlayerWidgetMagickaMode_OptionText", Label("uPlayerWidgetMagickaMode:PlayerWidget", ReadSetting("uPlayerWidgetMagickaMode:PlayerWidget") as Int), FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uPlayerWidgetMagickaMode:PlayerWidget"

        _types[index] = 3

        _help[index] = "$TrueHUD_PlayerWidgetMagickaMode_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_PlayerWidgetStaminaMode_OptionText", Label("uPlayerWidgetStaminaMode:PlayerWidget", ReadSetting("uPlayerWidgetStaminaMode:PlayerWidget") as Int), FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uPlayerWidgetStaminaMode:PlayerWidget"

        _types[index] = 3

        _help[index] = "$TrueHUD_PlayerWidgetStaminaMode_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_PlayerWidgetShoutIndicatorMode_OptionText", Label("uPlayerWidgetShoutIndicatorMode:PlayerWidget", ReadSetting("uPlayerWidgetShoutIndicatorMode:PlayerWidget") as Int), FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uPlayerWidgetShoutIndicatorMode:PlayerWidget"

        _types[index] = 3

        _help[index] = "$TrueHUD_PlayerWidgetShoutIndicatorMode_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_PlayerWidgetHideVanillaBars_OptionText", ReadSetting("bPlayerWidgetHideVanillaBars:PlayerWidget") != 0, FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bPlayerWidgetHideVanillaBars:PlayerWidget"

        _types[index] = 1

        _help[index] = "$TrueHUD_PlayerWidgetHideVanillaBars_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_PlayerWidgetForceHideVanillaBars_OptionText", ReadSetting("bPlayerWidgetForceHideVanillaBars:PlayerWidget") != 0, FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("bPlayerWidgetHideVanillaBars:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "bPlayerWidgetForceHideVanillaBars:PlayerWidget"

        _types[index] = 1

        _help[index] = "$TrueHUD_PlayerWidgetForceHideVanillaBars_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_PlayerWidgetDisplayEnchantmentChargeMeter_OptionText", ReadSetting("bPlayerWidgetDisplayEnchantmentChargeMeter:PlayerWidget") != 0, FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bPlayerWidgetDisplayEnchantmentChargeMeter:PlayerWidget"

        _types[index] = 1

        _help[index] = "$TrueHUD_PlayerWidgetDisplayEnchantmentChargeMeter_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_AlignmentSettings_HeaderText", 0)

    option = AddMenuOption("$TrueHUD_PlayerWidgetHealthBarDirection_OptionText", Label("uPlayerWidgetHealthBarDirection:PlayerWidget", ReadSetting("uPlayerWidgetHealthBarDirection:PlayerWidget") as Int), FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetHealthMode:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uPlayerWidgetHealthBarDirection:PlayerWidget"

        _types[index] = 3

        _help[index] = "$TrueHUD_PlayerWidgetHealthBarDirection_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_PlayerWidgetMagickaBarDirection_OptionText", Label("uPlayerWidgetMagickaBarDirection:PlayerWidget", ReadSetting("uPlayerWidgetMagickaBarDirection:PlayerWidget") as Int), FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetMagickaMode:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uPlayerWidgetMagickaBarDirection:PlayerWidget"

        _types[index] = 3

        _help[index] = "$TrueHUD_PlayerWidgetMagickaBarDirection_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_PlayerWidgetStaminaBarDirection_OptionText", Label("uPlayerWidgetStaminaBarDirection:PlayerWidget", ReadSetting("uPlayerWidgetStaminaBarDirection:PlayerWidget") as Int), FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetStaminaMode:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uPlayerWidgetStaminaBarDirection:PlayerWidget"

        _types[index] = 3

        _help[index] = "$TrueHUD_PlayerWidgetStaminaBarDirection_InfoText"

        _defaults[index] = 2.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_MiscSettings_HeaderText", 0)

    option = AddToggleOption("$TrueHUD_PlayerWidgetDisplayMountStamina_OptionText", ReadSetting("bPlayerWidgetDisplayMountStamina:PlayerWidget") != 0, FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetStaminaMode:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "bPlayerWidgetDisplayMountStamina:PlayerWidget"

        _types[index] = 1

        _help[index] = "$TrueHUD_PlayerWidgetDisplayMountStamina_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetPhantomDuration_OptionText", ReadSetting("fPlayerWidgetPhantomDuration:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("bPlayerWidgetDisplayPhantomBars:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetPhantomDuration:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetPhantomDuration_InfoText"

        _defaults[index] = 0.75

        _mins[index] = 0.01

        _maxs[index] = 5

        _steps[index] = 0.01

    EndIf

    AddHeaderOption("$TrueHUD_SpecialBar_HeaderText", 0)

    If ((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))

        AddTextOption("$TrueHUD_SpecialBars_OptionText", "$TrueHUD_SpecialBarsControlled_ValueText", 1)

    EndIf

    If ((ReadSetting("bEnablePlayerWidget:General") != 0) && !((ReadSetting("__SpecialResourcesControlled") != 0)))

        AddTextOption("$TrueHUD_SpecialBars_OptionText", "$TrueHUD_SpecialBarsNotControlled_ValueText", 1)

    EndIf

    option = AddToggleOption("$TrueHUD_PlayerWidgetDisplaySpecialPhantomBar_OptionText", ReadSetting("bPlayerWidgetDisplaySpecialPhantomBar:PlayerWidget") != 0, FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "bPlayerWidgetDisplaySpecialPhantomBar:PlayerWidget"

        _types[index] = 1

        _help[index] = "$TrueHUD_PlayerWidgetDisplaySpecialPhantomBar_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddMenuOption("$TrueHUD_PlayerWidgetSpecialMode_OptionText", Label("uPlayerWidgetSpecialMode:PlayerWidget", ReadSetting("uPlayerWidgetSpecialMode:PlayerWidget") as Int), FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("__SpecialResourcesControlled") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "uPlayerWidgetSpecialMode:PlayerWidget"

        _types[index] = 3

        _help[index] = "$TrueHUD_PlayerWidgetSpecialMode_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    SetCursorPosition(1)

    AddHeaderOption("$TrueHUD_DisplaySettings_HeaderText", 0)

    option = AddToggleOption("$TrueHUD_PlayerWidgetCombined_OptionText", ReadSetting("bPlayerWidgetCombined:PlayerWidget") != 0, FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bPlayerWidgetCombined:PlayerWidget"

        _types[index] = 1

        _help[index] = "$TrueHUD_PlayerWidgetCombined_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetScale_OptionText", ReadSetting("fPlayerWidgetScale:PlayerWidget"), "{2}", FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetScale:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetScale_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.01

        _maxs[index] = 1

        _steps[index] = 0.01

    EndIf

    option = AddToggleOption("$TrueHUD_PlayerWidgetUseHUDOpacity_OptionText", ReadSetting("bPlayerWidgetUseHUDOpacity:PlayerWidget") != 0, FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bPlayerWidgetUseHUDOpacity:PlayerWidget"

        _types[index] = 1

        _help[index] = "$TrueHUD_PlayerWidgetUseHUDOpacity_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetOpacity_OptionText", ReadSetting("fPlayerWidgetOpacity:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && !((ReadSetting("bPlayerWidgetUseHUDOpacity:PlayerWidget") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetOpacity:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetOpacity_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.01

        _maxs[index] = 1

        _steps[index] = 0.01

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetBarWidth_OptionText", ReadSetting("fPlayerWidgetBarWidth:PlayerWidget"), "{2}", FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetBarWidth:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetBarWidth_InfoText"

        _defaults[index] = 245.0

        _mins[index] = 50

        _maxs[index] = 1000

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetSpecialBarWidth_OptionText", ReadSetting("fPlayerWidgetSpecialBarWidth:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetSpecialMode:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetSpecialBarWidth:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetSpecialBarWidth_InfoText"

        _defaults[index] = 70.0

        _mins[index] = 30

        _maxs[index] = 1000

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_PlayerWidgetScaleBars_OptionText", ReadSetting("bPlayerWidgetScaleBars:PlayerWidget") != 0, FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bPlayerWidgetScaleBars:PlayerWidget"

        _types[index] = 1

        _help[index] = "$TrueHUD_PlayerWidgetScaleBars_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetMinBarWidth_OptionText", ReadSetting("fPlayerWidgetMinBarWidth:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("bPlayerWidgetScaleBars:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetMinBarWidth:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetMinBarWidth_InfoText"

        _defaults[index] = 50.0

        _mins[index] = 30

        _maxs[index] = 200

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetMaxBarWidth_OptionText", ReadSetting("fPlayerWidgetMaxBarWidth:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("bPlayerWidgetScaleBars:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetMaxBarWidth:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetMaxBarWidth_InfoText"

        _defaults[index] = 400.0

        _mins[index] = 100

        _maxs[index] = 1000

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetHealthBarScaleMult_OptionText", ReadSetting("fPlayerWidgetHealthBarScaleMult:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetHealthMode:PlayerWidget") != 0) && (ReadSetting("bPlayerWidgetScaleBars:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetHealthBarScaleMult:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetHealthBarScaleMult_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.1

        _maxs[index] = 10

        _steps[index] = 0.1

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetMagickaBarScaleMult_OptionText", ReadSetting("fPlayerWidgetMagickaBarScaleMult:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetMagickaMode:PlayerWidget") != 0) && (ReadSetting("bPlayerWidgetScaleBars:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetMagickaBarScaleMult:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetMagickaBarScaleMult_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.1

        _maxs[index] = 10

        _steps[index] = 0.1

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetStaminaBarScaleMult_OptionText", ReadSetting("fPlayerWidgetStaminaBarScaleMult:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetStaminaMode:PlayerWidget") != 0) && (ReadSetting("bPlayerWidgetScaleBars:PlayerWidget") != 0))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetStaminaBarScaleMult:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetStaminaBarScaleMult_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.1

        _maxs[index] = 10

        _steps[index] = 0.1

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetX_OptionText", ReadSetting("fPlayerWidgetX:PlayerWidget"), "{2}", FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetX:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetX_InfoText"

        _defaults[index] = 0.05

        _mins[index] = -0.5

        _maxs[index] = 1.5

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetY_OptionText", ReadSetting("fPlayerWidgetY:PlayerWidget"), "{2}", FlagsFor((ReadSetting("bEnablePlayerWidget:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetY:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetY_InfoText"

        _defaults[index] = 0.08

        _mins[index] = 0.001

        _maxs[index] = 1

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetHealthX_OptionText", ReadSetting("fPlayerWidgetHealthX:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetHealthMode:PlayerWidget") != 0) && !((ReadSetting("bPlayerWidgetCombined:PlayerWidget") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetHealthX:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetHealthX_InfoText"

        _defaults[index] = 0.5

        _mins[index] = -0.5

        _maxs[index] = 1.5

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetHealthY_OptionText", ReadSetting("fPlayerWidgetHealthY:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetHealthMode:PlayerWidget") != 0) && !((ReadSetting("bPlayerWidgetCombined:PlayerWidget") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetHealthY:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetHealthY_InfoText"

        _defaults[index] = 0.94

        _mins[index] = 0.001

        _maxs[index] = 1

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetMagickaX_OptionText", ReadSetting("fPlayerWidgetMagickaX:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetMagickaMode:PlayerWidget") != 0) && !((ReadSetting("bPlayerWidgetCombined:PlayerWidget") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetMagickaX:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetMagickaX_InfoText"

        _defaults[index] = 0.066

        _mins[index] = -0.5

        _maxs[index] = 1.5

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetMagickaY_OptionText", ReadSetting("fPlayerWidgetMagickaY:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetMagickaMode:PlayerWidget") != 0) && !((ReadSetting("bPlayerWidgetCombined:PlayerWidget") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetMagickaY:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetMagickaY_InfoText"

        _defaults[index] = 0.94

        _mins[index] = 0.001

        _maxs[index] = 1

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetStaminaX_OptionText", ReadSetting("fPlayerWidgetStaminaX:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetStaminaMode:PlayerWidget") != 0) && !((ReadSetting("bPlayerWidgetCombined:PlayerWidget") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetStaminaX:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetStaminaX_InfoText"

        _defaults[index] = 0.934

        _mins[index] = -0.5

        _maxs[index] = 1.5

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetStaminaY_OptionText", ReadSetting("fPlayerWidgetStaminaY:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("uPlayerWidgetStaminaMode:PlayerWidget") != 0) && !((ReadSetting("bPlayerWidgetCombined:PlayerWidget") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetStaminaY:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetStaminaY_InfoText"

        _defaults[index] = 0.94

        _mins[index] = 0.001

        _maxs[index] = 1

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetEnchantmentChargeMeterX_OptionText", ReadSetting("fPlayerWidgetEnchantmentChargeMeterX:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("bPlayerWidgetDisplayEnchantmentChargeMeter:PlayerWidget") != 0) && !((ReadSetting("bPlayerWidgetCombined:PlayerWidget") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetEnchantmentChargeMeterX:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetEnchantmentChargeMeterX_InfoText"

        _defaults[index] = 0.5

        _mins[index] = -0.5

        _maxs[index] = 1.5

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_PlayerWidgetEnchantmentChargeMeterY_OptionText", ReadSetting("fPlayerWidgetEnchantmentChargeMeterY:PlayerWidget"), "{2}", FlagsFor(((ReadSetting("bEnablePlayerWidget:General") != 0) && (ReadSetting("bPlayerWidgetDisplayEnchantmentChargeMeter:PlayerWidget") != 0) && !((ReadSetting("bPlayerWidgetCombined:PlayerWidget") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fPlayerWidgetEnchantmentChargeMeterY:PlayerWidget"

        _types[index] = 2

        _help[index] = "$TrueHUD_PlayerWidgetEnchantmentChargeMeterY_InfoText"

        _defaults[index] = 0.97

        _mins[index] = 0.001

        _maxs[index] = 1

        _steps[index] = 0.001

    EndIf

EndFunction

Function Page4()

    Int option

    Int index

    Float value

    AddHeaderOption("$TrueHUD_RecentLoot_HeaderText", 0)

    option = AddToggleOption("$TrueHUD_RecentLootHideVanillaMessage_OptionText", ReadSetting("bRecentLootHideVanillaMessage:RecentLoot") != 0, FlagsFor((ReadSetting("bEnableRecentLoot:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bRecentLootHideVanillaMessage:RecentLoot"

        _types[index] = 1

        _help[index] = "$TrueHUD_RecentLootHideVanillaMessage_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_RecentLootHideInInventoryMenus_OptionText", ReadSetting("bRecentLootHideInInventoryMenus:RecentLoot") != 0, FlagsFor((ReadSetting("bEnableRecentLoot:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bRecentLootHideInInventoryMenus:RecentLoot"

        _types[index] = 1

        _help[index] = "$TrueHUD_RecentLootHideInInventoryMenus_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddToggleOption("$TrueHUD_RecentLootHideInCraftingMenus_OptionText", ReadSetting("bRecentLootHideInCraftingMenus:RecentLoot") != 0, FlagsFor((ReadSetting("bEnableRecentLoot:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bRecentLootHideInCraftingMenus:RecentLoot"

        _types[index] = 1

        _help[index] = "$TrueHUD_RecentLootHideInCraftingMenus_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_RecentLootMaxMessageCount_OptionText", ReadSetting("uRecentLootMaxMessageCount:RecentLoot"), "{2}", FlagsFor((ReadSetting("bEnableRecentLoot:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uRecentLootMaxMessageCount:RecentLoot"

        _types[index] = 2

        _help[index] = "$TrueHUD_RecentLootMaxMessageCount_InfoText"

        _defaults[index] = 10.0

        _mins[index] = 1

        _maxs[index] = 20

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_RecentLootMessageDuration_OptionText", ReadSetting("fRecentLootMessageDuration:RecentLoot"), "{2}", FlagsFor((ReadSetting("bEnableRecentLoot:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fRecentLootMessageDuration:RecentLoot"

        _types[index] = 2

        _help[index] = "$TrueHUD_RecentLootMessageDuration_InfoText"

        _defaults[index] = 5.0

        _mins[index] = 0.5

        _maxs[index] = 20.0

        _steps[index] = 0.1

    EndIf

    option = AddSliderOption("$TrueHUD_RecentLootScale_OptionText", ReadSetting("fRecentLootScale:RecentLoot"), "{2}", FlagsFor((ReadSetting("bEnableRecentLoot:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fRecentLootScale:RecentLoot"

        _types[index] = 2

        _help[index] = "$TrueHUD_RecentLootScale_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.01

        _maxs[index] = 1

        _steps[index] = 0.01

    EndIf

    option = AddToggleOption("$TrueHUD_RecentLootUseHUDOpacity_OptionText", ReadSetting("bRecentLootUseHUDOpacity:RecentLoot") != 0, FlagsFor((ReadSetting("bEnableRecentLoot:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "bRecentLootUseHUDOpacity:RecentLoot"

        _types[index] = 1

        _help[index] = "$TrueHUD_RecentLootUseHUDOpacity_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_RecentLootOpacity_OptionText", ReadSetting("fRecentLootOpacity:RecentLoot"), "{2}", FlagsFor(((ReadSetting("bEnableRecentLoot:General") != 0) && !((ReadSetting("bRecentLootUseHUDOpacity:RecentLoot") != 0)))))

    If option >= 0

        index = option % 256

        _ids[index] = "fRecentLootOpacity:RecentLoot"

        _types[index] = 2

        _help[index] = "$TrueHUD_RecentLootOpacity_InfoText"

        _defaults[index] = 1.0

        _mins[index] = 0.01

        _maxs[index] = 1

        _steps[index] = 0.01

    EndIf

    option = AddMenuOption("$TrueHUD_RecentLootListDirection_OptionText", Label("uRecentLootListDirection:RecentLoot", ReadSetting("uRecentLootListDirection:RecentLoot") as Int), FlagsFor((ReadSetting("bEnableRecentLoot:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "uRecentLootListDirection:RecentLoot"

        _types[index] = 3

        _help[index] = "$TrueHUD_RecentLootListDirection_InfoText"

        _defaults[index] = 0.0

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddSliderOption("$TrueHUD_RecentLootX_OptionText", ReadSetting("fRecentLootX:RecentLoot"), "{2}", FlagsFor((ReadSetting("bEnableRecentLoot:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fRecentLootX:RecentLoot"

        _types[index] = 2

        _help[index] = "$TrueHUD_RecentLootX_InfoText"

        _defaults[index] = 1.0

        _mins[index] = -0.5

        _maxs[index] = 1.5

        _steps[index] = 0.001

    EndIf

    option = AddSliderOption("$TrueHUD_RecentLootY_OptionText", ReadSetting("fRecentLootY:RecentLoot"), "{2}", FlagsFor((ReadSetting("bEnableRecentLoot:General") != 0)))

    If option >= 0

        index = option % 256

        _ids[index] = "fRecentLootY:RecentLoot"

        _types[index] = 2

        _help[index] = "$TrueHUD_RecentLootY_InfoText"

        _defaults[index] = 0.85

        _mins[index] = 0.001

        _maxs[index] = 1

        _steps[index] = 0.001

    EndIf

EndFunction

Function Page5()

    Int option

    Int index

    Float value

    AddHeaderOption("$TrueHUD_HealthColors_HeaderText", 0)

    option = AddColorOption("$TrueHUD_HealthColor_OptionText", ReadSetting("sHealthColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sHealthColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_HealthColor_InfoText"

        _defaults[index] = 14622752

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_HealthPhantomColor_OptionText", ReadSetting("sHealthPhantomColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sHealthPhantomColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_HealthPhantomColor_InfoText"

        _defaults[index] = 13355979

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_HealthBackgroundColor_OptionText", ReadSetting("sHealthBackgroundColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sHealthBackgroundColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_HealthBackgroundColor_InfoText"

        _defaults[index] = 5050127

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_HealthPenaltyColor_OptionText", ReadSetting("sHealthPenaltyColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sHealthPenaltyColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_HealthPenaltyColor_InfoText"

        _defaults[index] = 6225920

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_HealthFlashColor_OptionText", ReadSetting("sHealthFlashColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sHealthFlashColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_HealthFlashColor_InfoText"

        _defaults[index] = 14622752

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_MagickaColors_HeaderText", 0)

    option = AddColorOption("$TrueHUD_MagickaColor_OptionText", ReadSetting("sMagickaColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sMagickaColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_MagickaColor_InfoText"

        _defaults[index] = 2640855

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_MagickaPhantomColor_OptionText", ReadSetting("sMagickaPhantomColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sMagickaPhantomColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_MagickaPhantomColor_InfoText"

        _defaults[index] = 13355979

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_MagickaBackgroundColor_OptionText", ReadSetting("sMagickaBackgroundColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sMagickaBackgroundColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_MagickaBackgroundColor_InfoText"

        _defaults[index] = 924237

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_MagickaPenaltyColor_OptionText", ReadSetting("sMagickaPenaltyColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sMagickaPenaltyColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_MagickaPenaltyColor_InfoText"

        _defaults[index] = 6225920

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_MagickaFlashColor_OptionText", ReadSetting("sMagickaFlashColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sMagickaFlashColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_MagickaFlashColor_InfoText"

        _defaults[index] = 2640855

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_StaminaColors_HeaderText", 0)

    option = AddColorOption("$TrueHUD_StaminaColor_OptionText", ReadSetting("sStaminaColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sStaminaColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_StaminaColor_InfoText"

        _defaults[index] = 32256

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_StaminaPhantomColor_OptionText", ReadSetting("sStaminaPhantomColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sStaminaPhantomColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_StaminaPhantomColor_InfoText"

        _defaults[index] = 13355979

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_StaminaBackgroundColor_OptionText", ReadSetting("sStaminaBackgroundColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sStaminaBackgroundColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_StaminaBackgroundColor_InfoText"

        _defaults[index] = 19712

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_StaminaPenaltyColor_OptionText", ReadSetting("sStaminaPenaltyColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sStaminaPenaltyColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_StaminaPenaltyColor_InfoText"

        _defaults[index] = 6225920

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_StaminaFlashColor_OptionText", ReadSetting("sStaminaFlashColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sStaminaFlashColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_StaminaFlashColor_InfoText"

        _defaults[index] = 32256

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    AddHeaderOption("$TrueHUD_SpecialColors_HeaderText", 0)

    option = AddColorOption("$TrueHUD_SpecialColor_OptionText", ReadSetting("sSpecialColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sSpecialColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_SpecialColor_InfoText"

        _defaults[index] = 16754176

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_SpecialPhantomColor_OptionText", ReadSetting("sSpecialPhantomColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sSpecialPhantomColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_SpecialPhantomColor_InfoText"

        _defaults[index] = 13355979

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_SpecialBackgroundColor_OptionText", ReadSetting("sSpecialBackgroundColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sSpecialBackgroundColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_SpecialBackgroundColor_InfoText"

        _defaults[index] = 8409856

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_SpecialPenaltyColor_OptionText", ReadSetting("sSpecialPenaltyColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sSpecialPenaltyColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_SpecialPenaltyColor_InfoText"

        _defaults[index] = 6225920

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_SpecialFlashColor_OptionText", ReadSetting("sSpecialFlashColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sSpecialFlashColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_SpecialFlashColor_InfoText"

        _defaults[index] = 16754176

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    SetCursorPosition(1)

    AddHeaderOption("$TrueHUD_MiscColors_HeaderText", 0)

    option = AddColorOption("$TrueHUD_DefaultColor_OptionText", ReadSetting("sDefaultColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sDefaultColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_DefaultColor_InfoText"

        _defaults[index] = 13355979

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_DefaultColorOutline_OptionText", ReadSetting("sDefaultColorOutline:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sDefaultColorOutline:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_DefaultColorOutline_InfoText"

        _defaults[index] = 13355979

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_WeakerColor_OptionText", ReadSetting("sWeakerColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sWeakerColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_WeakerColor_InfoText"

        _defaults[index] = 2001125

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_WeakerColorOutline_OptionText", ReadSetting("sWeakerColorOutline:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sWeakerColorOutline:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_WeakerColorOutline_InfoText"

        _defaults[index] = 6710886

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_StrongerColor_OptionText", ReadSetting("sStrongerColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sStrongerColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_StrongerColor_InfoText"

        _defaults[index] = 13379881

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_StrongerColorOutline_OptionText", ReadSetting("sStrongerColorOutline:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sStrongerColorOutline:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_StrongerColorOutline_InfoText"

        _defaults[index] = 13379881

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_TeammateColor_OptionText", ReadSetting("sTeammateColor:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sTeammateColor:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_TeammateColor_InfoText"

        _defaults[index] = 6736947

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

    option = AddColorOption("$TrueHUD_TeammateColorOutline_OptionText", ReadSetting("sTeammateColorOutline:Colors") as Int, 0)

    If option >= 0

        index = option % 256

        _ids[index] = "sTeammateColorOutline:Colors"

        _types[index] = 4

        _help[index] = "$TrueHUD_TeammateColorOutline_InfoText"

        _defaults[index] = 6736947

        _mins[index] = 0

        _maxs[index] = 1

        _steps[index] = 1

    EndIf

EndFunction

Int Function FlagsFor(Bool enabled)

    If enabled

        Return 0

    EndIf

    Return 1

EndFunction

String[] Function Choices(String id)

    String[] values

    If id == "uInfoBarDisplayHostiles:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_InfoBarDisplay_Never"

        values[1] = "$TrueHUD_InfoBarDisplay_OnHit"

        values[2] = "$TrueHUD_InfoBarDisplay_OnCombatStart"

    ElseIf id == "uInfoBarDisplayTeammates:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_InfoBarDisplay_Never"

        values[1] = "$TrueHUD_InfoBarDisplay_OnHit"

        values[2] = "$TrueHUD_InfoBarDisplay_OnCombatStart"

    ElseIf id == "uInfoBarDisplayOthers:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_InfoBarDisplay_Never"

        values[1] = "$TrueHUD_InfoBarDisplay_OnHit"

        values[2] = "$TrueHUD_InfoBarDisplay_OnCombatStart"

    ElseIf id == "uInfoBarDisplayName:ActorInfoBars"

        values = new String[5]

        values[0] = "$TrueHUD_InfoBarDisplayAttitude_Never"

        values[1] = "$TrueHUD_InfoBarDisplayAttitude_TargetOnly"

        values[2] = "$TrueHUD_InfoBarDisplayAttitude_Hostiles"

        values[3] = "$TrueHUD_InfoBarDisplayAttitude_HostilesAndTeammates"

        values[4] = "$TrueHUD_InfoBarDisplayAttitude_Everyone"

    ElseIf id == "uInfoBarDisplayDamageCounter:ActorInfoBars"

        values = new String[5]

        values[0] = "$TrueHUD_InfoBarDisplayAttitude_Never"

        values[1] = "$TrueHUD_InfoBarDisplayAttitude_TargetOnly"

        values[2] = "$TrueHUD_InfoBarDisplayAttitude_Hostiles"

        values[3] = "$TrueHUD_InfoBarDisplayAttitude_HostilesAndTeammates"

        values[4] = "$TrueHUD_InfoBarDisplayAttitude_Everyone"

    ElseIf id == "uInfoBarDisplayIndicator:ActorInfoBars"

        values = new String[5]

        values[0] = "$TrueHUD_InfoBarDisplayAttitude_Never"

        values[1] = "$TrueHUD_InfoBarDisplayAttitude_TargetOnly"

        values[2] = "$TrueHUD_InfoBarDisplayAttitude_Hostiles"

        values[3] = "$TrueHUD_InfoBarDisplayAttitude_HostilesAndTeammates"

        values[4] = "$TrueHUD_InfoBarDisplayAttitude_Everyone"

    ElseIf id == "uInfoBarResourcesModeTarget:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_ResourcesDisplayMode_Never"

        values[1] = "$TrueHUD_ResourcesDisplayMode_WhenChangedFromDefault"

        values[2] = "$TrueHUD_ResourcesDisplayMode_Always"

    ElseIf id == "uInfoBarResourcesModeHostiles:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_ResourcesDisplayMode_Never"

        values[1] = "$TrueHUD_ResourcesDisplayMode_WhenChangedFromDefault"

        values[2] = "$TrueHUD_ResourcesDisplayMode_Always"

    ElseIf id == "uInfoBarResourcesModeTeammates:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_ResourcesDisplayMode_Never"

        values[1] = "$TrueHUD_ResourcesDisplayMode_WhenChangedFromDefault"

        values[2] = "$TrueHUD_ResourcesDisplayMode_Always"

    ElseIf id == "uInfoBarResourcesModeOthers:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_ResourcesDisplayMode_Never"

        values[1] = "$TrueHUD_ResourcesDisplayMode_WhenChangedFromDefault"

        values[2] = "$TrueHUD_ResourcesDisplayMode_Always"

    ElseIf id == "uInfoBarSpecialModeTarget:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_ResourcesDisplayMode_Never"

        values[1] = "$TrueHUD_ResourcesDisplayMode_WhenChangedFromDefault"

        values[2] = "$TrueHUD_ResourcesDisplayMode_Always"

    ElseIf id == "uInfoBarSpecialModeHostiles:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_ResourcesDisplayMode_Never"

        values[1] = "$TrueHUD_ResourcesDisplayMode_WhenChangedFromDefault"

        values[2] = "$TrueHUD_ResourcesDisplayMode_Always"

    ElseIf id == "uInfoBarSpecialModeTeammates:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_ResourcesDisplayMode_Never"

        values[1] = "$TrueHUD_ResourcesDisplayMode_WhenChangedFromDefault"

        values[2] = "$TrueHUD_ResourcesDisplayMode_Always"

    ElseIf id == "uInfoBarSpecialModeOthers:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_ResourcesDisplayMode_Never"

        values[1] = "$TrueHUD_ResourcesDisplayMode_WhenChangedFromDefault"

        values[2] = "$TrueHUD_ResourcesDisplayMode_Always"

    ElseIf id == "uInfoBarDirection:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_BarDirection_Normal"

        values[1] = "$TrueHUD_BarDirection_Center"

        values[2] = "$TrueHUD_BarDirection_Reverse"

    ElseIf id == "uInfoBarDamageCounterAlignment:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_Alignment_Center"

        values[1] = "$TrueHUD_Alignment_Left"

        values[2] = "$TrueHUD_Alignment_Right"

    ElseIf id == "uInfoBarIndicatorMode:ActorInfoBars"

        values = new String[3]

        values[0] = "$TrueHUD_IndicatorMode_SoulIcon"

        values[1] = "$TrueHUD_IndicatorMode_LevelText"

        values[2] = "$TrueHUD_IndicatorMode_SquareDifficultyIcon"

    ElseIf id == "uInfoBarAnchor:ActorInfoBars"

        values = new String[2]

        values[0] = "$TrueHUD_WidgetAnchor_Body"

        values[1] = "$TrueHUD_WidgetAnchor_Head"

    ElseIf id == "uBossBarResourcesMode:BossBars"

        values = new String[3]

        values[0] = "$TrueHUD_ResourcesDisplayMode_Never"

        values[1] = "$TrueHUD_ResourcesDisplayMode_WhenChangedFromDefault"

        values[2] = "$TrueHUD_ResourcesDisplayMode_Always"

    ElseIf id == "uBossBarSpecialMode:BossBars"

        values = new String[3]

        values[0] = "$TrueHUD_ResourcesDisplayMode_Never"

        values[1] = "$TrueHUD_ResourcesDisplayMode_WhenChangedFromDefault"

        values[2] = "$TrueHUD_ResourcesDisplayMode_Always"

    ElseIf id == "uBossBarDirection:BossBars"

        values = new String[3]

        values[0] = "$TrueHUD_BarDirection_Normal"

        values[1] = "$TrueHUD_BarDirection_Center"

        values[2] = "$TrueHUD_BarDirection_Reverse"

    ElseIf id == "uBossBarDamageCounterAlignment:BossBars"

        values = new String[3]

        values[0] = "$TrueHUD_Alignment_Center"

        values[1] = "$TrueHUD_Alignment_Left"

        values[2] = "$TrueHUD_Alignment_Right"

    ElseIf id == "uBossBarResourceAlignment:BossBars"

        values = new String[3]

        values[0] = "$TrueHUD_Alignment_Center"

        values[1] = "$TrueHUD_Alignment_Left"

        values[2] = "$TrueHUD_Alignment_Right"

    ElseIf id == "uBossBarNameAlignment:BossBars"

        values = new String[3]

        values[0] = "$TrueHUD_Alignment_Center"

        values[1] = "$TrueHUD_Alignment_Left"

        values[2] = "$TrueHUD_Alignment_Right"

    ElseIf id == "uBossBarIndicatorMode:BossBars"

        values = new String[3]

        values[0] = "$TrueHUD_IndicatorMode_SoulIcon"

        values[1] = "$TrueHUD_IndicatorMode_LevelText"

        values[2] = "$TrueHUD_IndicatorMode_SquareDifficultyIcon"

    ElseIf id == "uBossBarModifyHUD:BossBars"

        values = new String[3]

        values[0] = "$TrueHUD_BossBarModifyHUD_None"

        values[1] = "$TrueHUD_BossBarModifyHUD_MoveSubtitles"

        values[2] = "$TrueHUD_BossBarModifyHUD_HideCompass"

    ElseIf id == "uMultipleBossBarsStackDirection:BossBars"

        values = new String[2]

        values[0] = "$TrueHUD_VerticalDirection_Up"

        values[1] = "$TrueHUD_VerticalDirection_Down"

    ElseIf id == "uPlayerWidgetHealthMode:PlayerWidget"

        values = new String[5]

        values[0] = "$TrueHUD_PlayerWidgetDisplayMode_Never"

        values[1] = "$TrueHUD_PlayerWidgetDisplayMode_Dynamic"

        values[2] = "$TrueHUD_PlayerWidgetDisplayMode_InCombat"

        values[3] = "$TrueHUD_PlayerWidgetDisplayMode_WhenAnotherDisplayed"

        values[4] = "$TrueHUD_PlayerWidgetDisplayMode_Always"

    ElseIf id == "uPlayerWidgetMagickaMode:PlayerWidget"

        values = new String[5]

        values[0] = "$TrueHUD_PlayerWidgetDisplayMode_Never"

        values[1] = "$TrueHUD_PlayerWidgetDisplayMode_Dynamic"

        values[2] = "$TrueHUD_PlayerWidgetDisplayMode_InCombat"

        values[3] = "$TrueHUD_PlayerWidgetDisplayMode_WhenAnotherDisplayed"

        values[4] = "$TrueHUD_PlayerWidgetDisplayMode_Always"

    ElseIf id == "uPlayerWidgetStaminaMode:PlayerWidget"

        values = new String[5]

        values[0] = "$TrueHUD_PlayerWidgetDisplayMode_Never"

        values[1] = "$TrueHUD_PlayerWidgetDisplayMode_Dynamic"

        values[2] = "$TrueHUD_PlayerWidgetDisplayMode_InCombat"

        values[3] = "$TrueHUD_PlayerWidgetDisplayMode_WhenAnotherDisplayed"

        values[4] = "$TrueHUD_PlayerWidgetDisplayMode_Always"

    ElseIf id == "uPlayerWidgetShoutIndicatorMode:PlayerWidget"

        values = new String[5]

        values[0] = "$TrueHUD_PlayerWidgetDisplayMode_Never"

        values[1] = "$TrueHUD_PlayerWidgetDisplayMode_Dynamic"

        values[2] = "$TrueHUD_PlayerWidgetDisplayMode_InCombat"

        values[3] = "$TrueHUD_PlayerWidgetDisplayMode_WhenAnotherDisplayed"

        values[4] = "$TrueHUD_PlayerWidgetDisplayMode_Always"

    ElseIf id == "uPlayerWidgetHealthBarDirection:PlayerWidget"

        values = new String[3]

        values[0] = "$TrueHUD_BarDirection_Normal"

        values[1] = "$TrueHUD_BarDirection_Center"

        values[2] = "$TrueHUD_BarDirection_Reverse"

    ElseIf id == "uPlayerWidgetMagickaBarDirection:PlayerWidget"

        values = new String[3]

        values[0] = "$TrueHUD_BarDirection_Normal"

        values[1] = "$TrueHUD_BarDirection_Center"

        values[2] = "$TrueHUD_BarDirection_Reverse"

    ElseIf id == "uPlayerWidgetStaminaBarDirection:PlayerWidget"

        values = new String[3]

        values[0] = "$TrueHUD_BarDirection_Normal"

        values[1] = "$TrueHUD_BarDirection_Center"

        values[2] = "$TrueHUD_BarDirection_Reverse"

    ElseIf id == "uPlayerWidgetSpecialMode:PlayerWidget"

        values = new String[5]

        values[0] = "$TrueHUD_PlayerWidgetDisplayMode_Never"

        values[1] = "$TrueHUD_PlayerWidgetDisplayMode_Dynamic"

        values[2] = "$TrueHUD_PlayerWidgetDisplayMode_InCombat"

        values[3] = "$TrueHUD_PlayerWidgetDisplayMode_WhenAnotherDisplayed"

        values[4] = "$TrueHUD_PlayerWidgetDisplayMode_Always"

    ElseIf id == "uRecentLootListDirection:RecentLoot"

        values = new String[2]

        values[0] = "$TrueHUD_VerticalDirection_Up"

        values[1] = "$TrueHUD_VerticalDirection_Down"

    EndIf

    Return values

EndFunction

String Function Label(String id, Int index)

    String[] values = Choices(id)

    If values && index >= 0 && index < values.Length

        Return values[index]

    EndIf

    Return ""

EndFunction

Event OnOptionSelect(Int option)

    Int index = option % 256

    If _types[index] == 1

        Float value = 1.0 - ReadSetting(_ids[index])

        WriteSetting(_ids[index], value)

        SetToggleOptionValue(option, value != 0, false)

        ForcePageReset()

    ElseIf _types[index] == 5

        ApplyPreset(_ids[index])

        ForcePageReset()

    EndIf

EndEvent

Event OnOptionSliderOpen(Int option)

    Int index = option % 256

    SetSliderDialogStartValue(ReadSetting(_ids[index]))

    SetSliderDialogDefaultValue(_defaults[index])

    SetSliderDialogRange(_mins[index], _maxs[index])

    SetSliderDialogInterval(_steps[index])

EndEvent

Event OnOptionSliderAccept(Int option, Float value)

    WriteSetting(_ids[option % 256], value)

    SetSliderOptionValue(option, ReadSetting(_ids[option % 256]), "{2}", false)

EndEvent

Event OnOptionMenuOpen(Int option)

    Int index = option % 256

    SetMenuDialogOptions(Choices(_ids[index]))

    SetMenuDialogStartIndex(ReadSetting(_ids[index]) as Int)

    SetMenuDialogDefaultIndex(_defaults[index] as Int)

EndEvent

Event OnOptionMenuAccept(Int option, Int value)

    Int index = option % 256

    WriteSetting(_ids[index], value)

    SetMenuOptionValue(option, Label(_ids[index], ReadSetting(_ids[index]) as Int), false)

    ForcePageReset()

EndEvent

Event OnOptionColorOpen(Int option)

    Int index = option % 256

    SetColorDialogStartColor(ReadSetting(_ids[index]) as Int)

    SetColorDialogDefaultColor(_defaults[index] as Int)

EndEvent

Event OnOptionColorAccept(Int option, Int value)

    WriteSetting(_ids[option % 256], value)

    SetColorOptionValue(option, ReadSetting(_ids[option % 256]) as Int, false)

EndEvent

Event OnOptionDefault(Int option)

    Int index = option % 256

    If _types[index] > 0 && _types[index] < 5

        WriteSetting(_ids[index], _defaults[index])

        ForcePageReset()

    EndIf

EndEvent

Event OnOptionHighlight(Int option)

    SetInfoText(_help[option % 256])

EndEvent

Function ApplyPreset(String a_presetName)
    if (a_presetName == "preset:BossBarTop")
        WriteSetting("fBossBarAnchorX:BossBars", 0.50)
        WriteSetting("fBossBarAnchorY:BossBars", 0.08)
        WriteSetting("uBossBarModifyHUD:BossBars", 2)
        WriteSetting("uMultipleBossBarsStackDirection:BossBars", 1)
        bBossBarsHideCompass = true
    elseif (a_presetName == "preset:BossBarBottom")
        WriteSetting("fBossBarAnchorX:BossBars", 0.50)
        WriteSetting("fBossBarAnchorY:BossBars", 0.92)
        WriteSetting("uBossBarModifyHUD:BossBars", 1)
        WriteSetting("uMultipleBossBarsStackDirection:BossBars", 0)
        bBossBarsHideCompass = false
    elseif (a_presetName == "preset:PlayerWidgetVanilla")
        WriteSetting("uPlayerWidgetHealthBarDirection:PlayerWidget", 1)
        WriteSetting("uPlayerWidgetMagickaBarDirection:PlayerWidget", 0)
        WriteSetting("uPlayerWidgetStaminaBarDirection:PlayerWidget", 2)
        WriteSetting("uPlayerWidgetHealthMode:PlayerWidget", 1)
        WriteSetting("uPlayerWidgetMagickaMode:PlayerWidget", 1)
        WriteSetting("uPlayerWidgetStaminaMode:PlayerWidget", 1)
        WriteSetting("uPlayerWidgetShoutIndicatorMode:PlayerWidget", 0)
        WriteSetting("bPlayerWidgetHideVanillaBars:PlayerWidget", 1)
        WriteSetting("bPlayerWidgetDisplayEnchantmentChargeMeter:PlayerWidget", 1)
        WriteSetting("fPlayerWidgetBarWidth:PlayerWidget", 245)
        WriteSetting("bPlayerWidgetScaleBars:PlayerWidget", 0)
        WriteSetting("bPlayerWidgetCombined:PlayerWidget", 0)
        WriteSetting("fPlayerWidgetHealthX:PlayerWidget", 0.500)
        WriteSetting("fPlayerWidgetHealthY:PlayerWidget", 0.940)
        WriteSetting("fPlayerWidgetMagickaX:PlayerWidget", 0.066)
        WriteSetting("fPlayerWidgetMagickaY:PlayerWidget", 0.940)
        WriteSetting("fPlayerWidgetStaminaX:PlayerWidget", 0.934)
        WriteSetting("fPlayerWidgetStaminaY:PlayerWidget", 0.940)
        WriteSetting("fPlayerWidgetEnchantmentChargeMeterX:PlayerWidget", 0.500)
        WriteSetting("fPlayerWidgetEnchantmentChargeMeterY:PlayerWidget", 0.970)
    elseif (a_presetName == "preset:PlayerWidgetCombined")
        WriteSetting("uPlayerWidgetHealthBarDirection:PlayerWidget", 0)
        WriteSetting("uPlayerWidgetMagickaBarDirection:PlayerWidget", 0)
        WriteSetting("uPlayerWidgetStaminaBarDirection:PlayerWidget", 0)
        WriteSetting("uPlayerWidgetHealthMode:PlayerWidget", 1)
        WriteSetting("uPlayerWidgetMagickaMode:PlayerWidget", 1)
        WriteSetting("uPlayerWidgetStaminaMode:PlayerWidget", 1)
        WriteSetting("uPlayerWidgetShoutIndicatorMode:PlayerWidget", 3)
        WriteSetting("bPlayerWidgetHideVanillaBars:PlayerWidget", 1)
        WriteSetting("bPlayerWidgetDisplayEnchantmentChargeMeter:PlayerWidget", 1)
        WriteSetting("fPlayerWidgetBarWidth:PlayerWidget", 100)
        WriteSetting("bPlayerWidgetScaleBars:PlayerWidget", 1)
        WriteSetting("fPlayerWidgetX:PlayerWidget", 0.045)
        WriteSetting("fPlayerWidgetY:PlayerWidget", 0.920)
        WriteSetting("bPlayerWidgetCombined:PlayerWidget", 1)
        WriteSetting("fPlayerWidgetMinBarWidth:PlayerWidget", 50)
        WriteSetting("fPlayerWidgetMaxBarWidth:PlayerWidget", 400)
    endif
    ForcePageReset()
EndFunction

