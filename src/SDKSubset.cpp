// Small unchanged definitions from SKSE 1.7.3 GameReferences.cpp and
// ScaleformLoader.cpp. Kept separate to avoid linking unrelated SKSE hooks,
// translation importers and model mutation code into a plugin.
// Original SKSE authors' source/license must accompany the release.
#include "Runtime.h"
#include "skse/GameData.h"

const _CreateRefHandleByREFR CreateRefHandleByREFR = (_CreateRefHandleByREFR)0x0065CC00;
const _LookupREFRByHandle LookupREFRByHandle = (_LookupREFRByHandle)0x004A9180;
const UInt32* g_invalidRefHandle = (UInt32*)0x01310630;
const _LookupFormByID LookupFormByID = (_LookupFormByID)0x00451A30;
const _HasLOS HasLOS = (_HasLOS)0x008C0FA0;

UInt32 TESObjectREFR::CreateRefHandle() {
    if (handleRefObject.GetRefCount() > 0) {
        UInt32 result = 0;
        CreateRefHandleByREFR(&result, this);
        return result;
    }
    return *g_invalidRefHandle;
}
GFxLoader* GFxLoader::GetSingleton() {
    return *((GFxLoader**)0x01B2E9B0);
}
