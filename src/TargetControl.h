#pragma once
#include "SpecialResources.h"
namespace TrueHUDLE {
    ResourceResult RequestTargetControlNative(UInt32 owner);
    ResourceResult ReleaseTargetControlNative(UInt32 owner);
    ResourceResult SetControlledTarget(UInt32 owner, UInt32 handle, bool soft);
    UInt32 ReadTargetControlOwner();
}
