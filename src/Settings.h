#pragma once
#include "SettingsDefaults.h"
#include <windows.h>
namespace TrueHUDLE {
    bool InitializeSettings();
    void CopySettings(Settings& output);
    double GetSetting(const char* id);
    bool SetSetting(const char* id, double value);
    bool ReadSettings();
    bool WriteSettings();
    bool ParseNumber(const char* text, double& result, bool hex = false);
}
