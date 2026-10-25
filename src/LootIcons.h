#pragma once
#include "Runtime.h"
namespace TrueHUDLE {
    enum LootKeyword : UInt32 {
        Clothing=1u<<0, Jewelry=1u<<1, Recipe=1u<<2, SpellTome=1u<<3,
        ChildClothes=1u<<4, Artifact=1u<<5, Gem=1u<<6, Hide=1u<<7,
        Remains=1u<<8, Ingot=1u<<9, Clutter=1u<<10, Firewood=1u<<11, Warhammer=1u<<12
    };
    struct LootTraits {
        UInt32 type=0, formID=0, keywords=0, parts=0, weight=2, subtype=0, flags=0, primaryAV=0xFFFFFFFFu;
        UInt32 capacity=0, soul=0, useSound=0;
        bool playable=true;
    };
    void ClassifyLootTraits(const LootTraits& traits, LootSample& output);
    bool ClassifyLoot(TESForm* form, LootSample& output);
}
