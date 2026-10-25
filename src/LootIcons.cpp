// Icon policy adapted from Ersh TrueHUD 1.1.10 RecentLoot.cpp (GPL + EXCEPTIONS).
// All native field reads use the LE 1.7.3 SDK, never SE structures.
#include "LootIcons.h"
#include "skse/GameObjects.h"
#include <cstring>

namespace TrueHUDLE {
    namespace {
        void Icon(LootSample& item, const char* label, UInt32 color=0xFFFFFF) {
            strcpy_s(item.icon,label); item.iconColor=color;
        }
        UInt32 Keywords(const BGSKeywordForm& form) {
            if (form.numKeywords>4096 || !Readable(form.keywords,form.numKeywords*sizeof(BGSKeyword*))) return 0;
            struct Entry { const char* name; UInt32 bit; };
            const Entry names[]={{"VendorItemClothing",Clothing},{"VendorItemJewelry",Jewelry},{"VendorItemRecipe",Recipe},
                {"VendorItemSpellTome",SpellTome},{"BYOHAdoptionClothesKeyword",ChildClothes},{"VendorItemDaedricArtifact",Artifact},
                {"VendorItemGem",Gem},{"VendorItemAnimalHide",Hide},{"VendorItemAnimalPart",Remains},{"VendorItemOreIngot",Ingot},
                {"VendorItemClutter",Clutter},{"VendorItemFirewood",Firewood},{"WeapTypeWarhammer",Warhammer}};
            UInt32 mask=0;
            for (UInt32 i=0;i<form.numKeywords;++i) {
                auto keyword=form.keywords[i];
                if (!Readable(keyword,sizeof(BGSKeyword))) continue;
                char name[64]; CopyString(keyword->keyword.Get(),name,sizeof(name));
                for (const auto& entry:names) if (!strcmp(name,entry.name)) { mask|=entry.bit; break; }
            }
            return mask;
        }
        template<class T> T* Form(TESForm* form) { return Readable(form,sizeof(T)) ? static_cast<T*>(form) : nullptr; }
        bool Traits(TESForm* form, LootTraits& t) {
            if (!Readable(form,sizeof(TESForm))) return false;
            t.type=form->formType; t.formID=form->formID;
            // Native TESForm00401A50, weapon004A8E90, ammo00495BA0:
            // the latter two override the generic record-flag predicate.
            t.playable=(form->flags & 4)==0;
            switch (t.type) {
            case kFormType_Armor: {
                auto item=Form<TESObjectARMO>(form); if (!item) return false;
                t.keywords=Keywords(item->keyword); t.parts=item->bipedObject.data.parts; t.weight=item->bipedObject.data.weightClass;
                break;
            }
            case kFormType_Weapon: {
                auto item=Form<TESObjectWEAP>(form); if (!item) return false;
                t.keywords=Keywords(item->keyword); t.subtype=item->gameData.type;
                t.playable=(item->gameData.flags2 & TESObjectWEAP::GameData::kFlags_NotPlayable)==0; break;
            }
            case kFormType_Ammo: {
                auto item=Form<TESAmmo>(form); if (!item) return false;
                t.flags=item->settings.flags; t.playable=(t.flags & TESAmmo::kNotPlayable)==0; break;
            }
            case kFormType_Book: {
                auto item=Form<TESObjectBOOK>(form); if (!item) return false;
                t.keywords=Keywords(item->keyword); t.subtype=item->data.type;
                break;
            }
            case kFormType_Misc: {
                auto item=Form<TESObjectMISC>(form); if (!item) return false;
                t.keywords=Keywords(item->keyword); break;
            }
            case kFormType_Potion: {
                auto item=Form<AlchemyItem>(form); if (!item) return false;
                t.flags=item->itemData.flags;
                if (Readable(item->itemData.useSound,sizeof(TESForm))) t.useSound=item->itemData.useSound->formID;
                EffectSetting* effect=item->unk44;
                const auto& list=item->effectItemList;
                if (!Readable(effect,sizeof(EffectSetting)) && list.count && list.count<=4096 && list.count<=list.arr.capacity &&
                    Readable(list.arr.entries,list.count*sizeof(MagicItem::EffectItem*))) {
                    auto first=list.arr.entries[0];
                    if (Readable(first,sizeof(MagicItem::EffectItem))) effect=first->mgef;
                }
                if (Readable(effect,sizeof(EffectSetting))) t.primaryAV=effect->properties.primaryValue;
                break;
            }
            case kFormType_SoulGem: {
                auto item=Form<TESSoulGem>(form); if (!item) return false;
                t.capacity=item->gemSize; t.soul=item->soulSize; break;
            }
            case kFormType_ScrollItem: case kFormType_Ingredient: case kFormType_Light: case kFormType_Key: break;
            default: return false;
            }
            return t.playable;
        }
    }
    void ClassifyLootTraits(const LootTraits& t, LootSample& out) {
        Icon(out,"default_misc");
        // Hardcoded Skyrim identities are full IDs, so another plugin's local
        // ID cannot turn an unrelated item into gold, a claw or Azura's Star.
        switch (t.type) {
        case kFormType_ScrollItem: Icon(out,"default_scroll"); break;
        case kFormType_Ingredient: Icon(out,"default_ingredient"); break;
        case kFormType_Light: Icon(out,"misc_torch"); break;
        case kFormType_Key: Icon(out,"default_key"); break;
        case kFormType_Armor: {
            const unsigned order[]={2,1,3,4,7,8,9,5,6,11,13,0,12,10};
            unsigned part=0; bool found=false;
            for (unsigned bit:order) if (t.parts & (1u<<bit)) { part=bit; found=true; break; }
            // Only modder slots (or no slots) retain the upstream head fallback.
            if (!found) part=0;
            const char* suffix=part==0 || part==1 || part==11 ? "head" : part==2 || part==10 ? "body" :
                part==3 ? "hands" : part==4 ? "forearms" : part==7 ? "feet" : part==8 ? "calves" :
                part==9 ? "shield" : part==5 ? "amulet" : part==6 ? "ring" : part==12 ? "circlet" : nullptr;
            const bool jewelry=t.formID==0x8895A || (t.weight>1 && !(t.keywords & Clothing) &&
                ((t.keywords & Jewelry) || part==5 || part==6 || part==12 || part==13));
            const UInt32 color=t.weight==0 ? 0x756000 : t.weight==1 ? 0x6B7585 : 0xEDDA87;
            Icon(out,"default_armor",color);
            if (!suffix) break;
            if (part==5 || part==6 || part==12) {
                if (t.weight<2 || jewelry) sprintf_s(out.icon,"armor_%s",suffix);
            } else if (!jewelry) sprintf_s(out.icon,"%s_%s",t.weight==0 ? "lightarmor" : t.weight==1 ? "armor" : "clothing",suffix);
            break;
        }
        case kFormType_Book:
            Icon(out,t.subtype==0xFF || (t.keywords & Recipe) ? "book_note" : (t.keywords & SpellTome) ? "book_tome" : "default_book"); break;
        case kFormType_Weapon: {
            const char* types[]={"default_weapon","weapon_sword","weapon_dagger","weapon_waraxe","weapon_mace",
                "weapon_greatsword","weapon_battleaxe","weapon_bow","weapon_staff","weapon_crossbow"};
            Icon(out,t.subtype<10 ? types[t.subtype] : "default_weapon",0xA4A5BF);
            if (t.subtype==6 && (t.keywords & Warhammer)) Icon(out,"weapon_hammer",0xA4A5BF);
            if (t.formID==0xE3C16 || t.formID==0x6A707 || t.formID==0x1019D4) Icon(out,"weapon_pickaxe",0xA4A5BF);
            if (t.formID==0x2F2F4 || t.formID==0xAE086) Icon(out,"weapon_woodaxe",0xA4A5BF);
            break;
        }
        case kFormType_Ammo: Icon(out,(t.flags & TESAmmo::kNotBolt) ? "weapon_arrow" : "weapon_bolt",0xA89E8C); break;
        case kFormType_Misc: {
            if (t.keywords & ChildClothes) Icon(out,"clothing_body",0xEDDA87);
            else if (t.keywords & Artifact) Icon(out,"misc_artifact");
            else if (t.keywords & Gem) Icon(out,"misc_gem",0xFFB0D1);
            else if (t.keywords & Hide) Icon(out,"misc_hide",0xDBB36E);
            else if (t.keywords & Remains) Icon(out,"misc_remains");
            else if (t.keywords & Ingot) Icon(out,"misc_ingot",0x828282);
            else if (t.keywords & Clutter) Icon(out,"misc_clutter");
            else if (t.keywords & Firewood) Icon(out,"misc_wood",0xA89E8C);
            switch (t.formID) {
            case 0x6851E: Icon(out,"misc_gem",0xFFB0D1); break;
            case 0x4B56C: case 0xAB7BB: case 0x7C260: case 0x5AF48: case 0xED417: case 0xAB375:
            case 0x8CDFA: case 0xB634C: case 0x999E7: case 0x663D7: case 0x39647: Icon(out,"misc_dragonclaw"); break;
            case 0xA: Icon(out,"misc_lockpick"); break;
            case 0xF: Icon(out,"misc_gold",0xCCCC33); break;
            case 0xDB5D2: Icon(out,"misc_leather",0xBA8D23); break;
            case 0x800E4: Icon(out,"misc_strips",0xBA8D23); break;
            }
            break;
        }
        case kFormType_Potion:
            if (t.flags & AlchemyItem::kFlag_Food) { Icon(out,t.useSound==0xB6435 ? "food_wine" : "default_food"); break; }
            if (t.flags & AlchemyItem::kFlag_Poison) { Icon(out,"potion_poison",0xAD00B3); break; }
            Icon(out,"default_potion");
            // LE registration005AD830, evidence/ActorValues-registration.c.
            switch (t.primaryAV) {
            case 24: case 27: case 155: Icon(out,"potion_health",0xDB2E73); break;
            case 25: case 28: case 156: Icon(out,"potion_magic",0x2E9FDB); break;
            case 26: case 29: case 157: Icon(out,"potion_stam",0x51DB2E); break;
            case 41: Icon(out,"potion_fire",0xC73636); break;
            case 42: Icon(out,"potion_shock",0xEAAB00); break;
            case 43: Icon(out,"potion_frost",0x1FFBFF); break;
            }
            break;
        case kFormType_SoulGem: {
            const UInt32 colors[]={0xE3E0FF,0xD7D4FF,0xC0BAFF,0xABA3FF,0x948BFC,0x7569FF};
            const unsigned status=!t.capacity || !t.soul ? 0 : t.soul>=t.capacity ? 2 : 1;
            const char* smallGemIcons[]={"soulgem_empty","soulgem_partial","soulgem_full"};
            const char* grandGemIcons[]={"soulgem_grandempty","soulgem_grandpartial","soulgem_grandfull"};
            Icon(out,t.capacity>0 && t.capacity<=5 ? (t.capacity>=4 ? grandGemIcons[status] : smallGemIcons[status]) : "misc_soulgem",
                t.capacity<=5 ? colors[t.capacity] : colors[0]);
            if (t.formID==0x63B27 || t.formID==0x63B29) Icon(out,"soulgem_azura",0x7569FF);
            break;
        }
        }
    }
    bool ClassifyLoot(TESForm* form, LootSample& output) {
        LootTraits traits;
        if (!Traits(form,traits)) return false;
        ClassifyLootTraits(traits,output); return true;
    }
}
