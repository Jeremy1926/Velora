#include "pch.h"
#include "Athena/FortPoiVolume.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

bool AFortPoiVolume::OverlapsPawn(APawn* Pawn) const
{
    DETOUR(AFortPoiVolume, OverlapsPawn, Anchor::Hook()
        .String(TEXT("STAT_PoiVolume_CheckPawnOverlap"))
        .Optional()
        .Find("call 0x*")
        .Target()
        .Inline());

    if (!GetBrushComponent())
        return false;

    return Original(Pawn);
}

UObject* AFortPoiVolume::GetBrushComponent() const
{
    static auto BrushComponent = GetClass()->FindPropertyByName(FName(TEXT("BrushComponent")));

    return *BrushComponent->ContainerPtrToValuePtr<UObject*>(this);
}
