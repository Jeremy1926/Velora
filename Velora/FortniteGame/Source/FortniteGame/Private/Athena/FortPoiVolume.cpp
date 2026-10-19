#include "pch.h"
#include "Athena/FortPoiVolume.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

static auto OverlapsPawn = []
{
    auto CheckPawnOverlap = Scanner::String(TEXT("STAT_PoiVolume_CheckPawnOverlap"));

    return CheckPawnOverlap.Get() ? Detour(TEXT("AFortPoiVolume::OverlapsPawn"), CheckPawnOverlap.Find("call 0x*").Target(), &AFortPoiVolume::OverlapsPawn) : nullptr;
}();

bool AFortPoiVolume::OverlapsPawn(APawn* Pawn) const
{
    if (!GetBrushComponent())
        return false;

    return (this->*::OverlapsPawn)(Pawn);
}

UObject* AFortPoiVolume::GetBrushComponent() const
{
    static auto BrushComponent = GetClass()->FindPropertyByName(FName(TEXT("BrushComponent")));

    return *BrushComponent->ContainerPtrToValuePtr<UObject*>(this);
}
