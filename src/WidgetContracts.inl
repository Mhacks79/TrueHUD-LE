// Generated argument contracts from upstream Widgets/*.cpp; GPL + EXCEPTIONS.

// Engine behavior is implemented separately for LE.

void ConfigureActorInfoBar(GFxValue& object, const Settings& settings, float opacity, float width, float height, bool specialMode = false, bool specialAvailable = false) {
GFxValue args[44];
		args[0].SetNumber(static_cast<uint32_t>(settings.uInfoBarDirection));
		args[1].SetNumber(static_cast<uint32_t>(settings.uInfoBarDisplayName));
		args[2].SetBool(settings.bInfoBarDisplayPhantomBars);
		args[3].SetBool(settings.bInfoBarDisplaySpecialPhantomBar);
		args[4].SetNumber(static_cast<uint32_t>(settings.uInfoBarDisplayDamageCounter));
		args[5].SetNumber(static_cast<uint32_t>(settings.uInfoBarDisplayIndicator));
		args[6].SetNumber(static_cast<uint32_t>(settings.uInfoBarDamageCounterAlignment));
		args[7].SetNumber(static_cast<uint32_t>(settings.uInfoBarResourcesModeTarget));
		args[8].SetNumber(static_cast<uint32_t>(settings.uInfoBarResourcesModeHostiles));
		args[9].SetNumber(static_cast<uint32_t>(settings.uInfoBarResourcesModeTeammates));
		args[10].SetNumber(static_cast<uint32_t>(settings.uInfoBarResourcesModeOthers));
		args[11].SetNumber(static_cast<uint32_t>((specialAvailable ? settings.uInfoBarSpecialModeTarget : 0)));
		args[12].SetNumber(static_cast<uint32_t>((specialAvailable ? settings.uInfoBarSpecialModeHostiles : 0)));
		args[13].SetNumber(static_cast<uint32_t>((specialAvailable ? settings.uInfoBarSpecialModeTeammates : 0)));
		args[14].SetNumber(static_cast<uint32_t>((specialAvailable ? settings.uInfoBarSpecialModeOthers : 0)));
		args[15].SetBool(specialMode);
		args[16].SetNumber(static_cast<uint32_t>(settings.uInfoBarIndicatorMode));	
		args[17].SetNumber(settings.fInfoBarPhantomDuration);
		args[18].SetNumber(settings.fInfoBarDamageCounterDuration);
		args[19].SetNumber((settings.bInfoBarUseHUDOpacity ? opacity : settings.fInfoBarOpacity) * 100.f);
		args[20].SetNumber(settings.fInfoBarWidth);
		args[21].SetNumber(settings.fInfoBarResourceWidth);
		args[22].SetNumber(settings.fInfoBarTargetWidth);
		args[23].SetNumber(settings.fInfoBarTargetResourceWidth);
		args[24].SetNumber(settings.uHealthColor);
		args[25].SetNumber(settings.uHealthPhantomColor);
		args[26].SetNumber(settings.uHealthBackgroundColor);
		args[27].SetNumber(settings.uHealthPenaltyColor);
		args[28].SetNumber(settings.uHealthFlashColor);
		args[29].SetNumber(settings.uMagickaColor);
		args[30].SetNumber(settings.uMagickaPhantomColor);
		args[31].SetNumber(settings.uMagickaBackgroundColor);
		args[32].SetNumber(settings.uMagickaPenaltyColor);
		args[33].SetNumber(settings.uMagickaFlashColor);
		args[34].SetNumber(settings.uStaminaColor);
		args[35].SetNumber(settings.uStaminaPhantomColor);
		args[36].SetNumber(settings.uStaminaBackgroundColor);
		args[37].SetNumber(settings.uStaminaPenaltyColor);
		args[38].SetNumber(settings.uStaminaFlashColor);
		args[39].SetNumber(settings.uSpecialColor);
		args[40].SetNumber(settings.uSpecialPhantomColor);
		args[41].SetNumber(settings.uSpecialBackgroundColor);
		args[42].SetNumber(settings.uSpecialPenaltyColor);
		args[43].SetNumber(settings.uSpecialFlashColor);
		object.Invoke("loadConfig", nullptr, args, 44);
}

void ConfigureBossInfoBar(GFxValue& object, const Settings& settings, float opacity, float width, float height, bool specialMode = false, bool specialAvailable = false) {
GFxValue args[37];
		args[0].SetNumber(static_cast<uint32_t>(settings.uBossBarDirection));
		args[1].SetBool(settings.bBossBarDisplayPhantomBars);
		args[2].SetBool(settings.bBossBarDisplaySpecialPhantomBar);
		args[3].SetBool(settings.bBossBarDisplayDamageCounter);
		args[4].SetBool(settings.bBossBarDisplayIndicator);
		args[5].SetNumber(static_cast<uint32_t>(settings.uBossBarDamageCounterAlignment));
		args[6].SetNumber(static_cast<uint32_t>(settings.uBossBarResourcesMode));
		args[7].SetNumber(static_cast<uint32_t>((specialAvailable ? settings.uBossBarSpecialMode : 0)));
		args[8].SetBool(specialMode);
		args[9].SetNumber(static_cast<uint32_t>(settings.uBossBarResourceAlignment));
		args[10].SetNumber(static_cast<uint32_t>(settings.uBossBarNameAlignment));
		args[11].SetNumber(static_cast<uint32_t>(settings.uBossBarIndicatorMode));
		args[12].SetNumber(settings.fBossBarPhantomDuration);
		args[13].SetNumber(settings.fBossBarDamageCounterDuration);
		args[14].SetNumber((settings.bBossBarUseHUDOpacity ? opacity : settings.fBossBarOpacity) * 100.f);
		args[15].SetNumber(settings.fBossBarWidth);
		args[16].SetNumber(settings.fBossBarResourceWidth);
		args[17].SetNumber(settings.uHealthColor);
		args[18].SetNumber(settings.uHealthPhantomColor);
		args[19].SetNumber(settings.uHealthBackgroundColor);
		args[20].SetNumber(settings.uHealthPenaltyColor);
		args[21].SetNumber(settings.uHealthFlashColor);
		args[22].SetNumber(settings.uMagickaColor);
		args[23].SetNumber(settings.uMagickaPhantomColor);
		args[24].SetNumber(settings.uMagickaBackgroundColor);
		args[25].SetNumber(settings.uMagickaPenaltyColor);
		args[26].SetNumber(settings.uMagickaFlashColor);
		args[27].SetNumber(settings.uStaminaColor);
		args[28].SetNumber(settings.uStaminaPhantomColor);
		args[29].SetNumber(settings.uStaminaBackgroundColor);
		args[30].SetNumber(settings.uStaminaPenaltyColor);
		args[31].SetNumber(settings.uStaminaFlashColor);
		args[32].SetNumber(settings.uSpecialColor);
		args[33].SetNumber(settings.uSpecialPhantomColor);
		args[34].SetNumber(settings.uSpecialBackgroundColor);
		args[35].SetNumber(settings.uSpecialPenaltyColor);
		args[36].SetNumber(settings.uSpecialFlashColor);
		object.Invoke("loadConfig", nullptr, args, 37);
}

void ConfigurePlayerWidget(GFxValue& object, const Settings& settings, float opacity, float width, float height, bool specialMode = false, bool specialAvailable = false) {
GFxValue args[54];
		args[0].SetNumber(static_cast<uint32_t>(settings.uPlayerWidgetHealthBarDirection));
		args[1].SetNumber(static_cast<uint32_t>(settings.uPlayerWidgetMagickaBarDirection));
		args[2].SetNumber(static_cast<uint32_t>(settings.uPlayerWidgetStaminaBarDirection));
		args[3].SetBool(settings.bPlayerWidgetDisplayPhantomBars);
		args[4].SetBool(settings.bPlayerWidgetDisplaySpecialPhantomBar);
		args[5].SetNumber(static_cast<uint32_t>(settings.uPlayerWidgetHealthMode));
		args[6].SetNumber(static_cast<uint32_t>(settings.uPlayerWidgetMagickaMode));
		args[7].SetNumber(static_cast<uint32_t>(settings.uPlayerWidgetStaminaMode));
		args[8].SetNumber(static_cast<uint32_t>(settings.uPlayerWidgetShoutIndicatorMode));
		args[9].SetNumber(static_cast<uint32_t>((specialAvailable ? settings.uPlayerWidgetSpecialMode : 0)));
		args[10].SetBool(specialMode);
		args[11].SetBool(settings.bPlayerWidgetDisplayEnchantmentChargeMeter);
		args[12].SetNumber(settings.fPlayerWidgetPhantomDuration);
		args[13].SetNumber(settings.fPlayerWidgetScale);
		args[14].SetNumber((settings.bPlayerWidgetUseHUDOpacity ? opacity : settings.fPlayerWidgetOpacity) * 100.f);
		args[15].SetNumber(settings.fPlayerWidgetBarWidth);
		args[16].SetNumber(settings.fPlayerWidgetSpecialBarWidth);
		args[17].SetBool(settings.bPlayerWidgetScaleBars);
		args[18].SetNumber(settings.fPlayerWidgetMinBarWidth);
		args[19].SetNumber(settings.fPlayerWidgetMaxBarWidth);
		args[20].SetNumber(settings.fPlayerWidgetHealthBarScaleMult);
		args[21].SetNumber(settings.fPlayerWidgetMagickaBarScaleMult);
		args[22].SetNumber(settings.fPlayerWidgetStaminaBarScaleMult);
		args[23].SetNumber(width * settings.fPlayerWidgetX);
		args[24].SetNumber(height * settings.fPlayerWidgetY);
		args[25].SetNumber(width * settings.fPlayerWidgetEnchantmentChargeMeterX);
		args[26].SetNumber(height * settings.fPlayerWidgetEnchantmentChargeMeterY);
		args[27].SetNumber(settings.bPlayerWidgetCombined);
		args[28].SetNumber(width * settings.fPlayerWidgetHealthX);
		args[29].SetNumber(height * settings.fPlayerWidgetHealthY);
		args[30].SetNumber(width * settings.fPlayerWidgetMagickaX);
		args[31].SetNumber(height * settings.fPlayerWidgetMagickaY);
		args[32].SetNumber(width * settings.fPlayerWidgetStaminaX);
		args[33].SetNumber(height * settings.fPlayerWidgetStaminaY);
		args[34].SetNumber(settings.uHealthColor);
		args[35].SetNumber(settings.uHealthPhantomColor);
		args[36].SetNumber(settings.uHealthBackgroundColor);
		args[37].SetNumber(settings.uHealthPenaltyColor);
		args[38].SetNumber(settings.uHealthFlashColor);
		args[39].SetNumber(settings.uMagickaColor);
		args[40].SetNumber(settings.uMagickaPhantomColor);
		args[41].SetNumber(settings.uMagickaBackgroundColor);
		args[42].SetNumber(settings.uMagickaPenaltyColor);
		args[43].SetNumber(settings.uMagickaFlashColor);
		args[44].SetNumber(settings.uStaminaColor);
		args[45].SetNumber(settings.uStaminaPhantomColor);
		args[46].SetNumber(settings.uStaminaBackgroundColor);
		args[47].SetNumber(settings.uStaminaPenaltyColor);
		args[48].SetNumber(settings.uStaminaFlashColor);
		args[49].SetNumber(settings.uSpecialColor);
		args[50].SetNumber(settings.uSpecialPhantomColor);
		args[51].SetNumber(settings.uSpecialBackgroundColor);
		args[52].SetNumber(settings.uSpecialPenaltyColor);
		args[53].SetNumber(settings.uSpecialFlashColor);
		object.Invoke("loadConfig", nullptr, args, 54);
}

void ConfigureRecentLoot(GFxValue& object, const Settings& settings, float opacity, float width, float height) {
GFxValue args[4];
		args[0].SetNumber(settings.uRecentLootMaxMessageCount);
		args[1].SetNumber(settings.fRecentLootMessageDuration);
		args[2].SetNumber((settings.bRecentLootUseHUDOpacity ? opacity : settings.fRecentLootOpacity) * 100.f);
		args[3].SetNumber(static_cast<uint32_t>(settings.uRecentLootListDirection));
		object.Invoke("loadConfig", nullptr, args, 4);
}

void ConfigureShoutIndicator(GFxValue& object, const Settings& settings, float opacity, float width, float height) {
GFxValue arg;
		arg.SetNumber((settings.bBossBarUseHUDOpacity ? opacity : settings.fBossBarOpacity) * 100.f);
		object.Invoke("loadConfig", nullptr, &arg, 1);
}
