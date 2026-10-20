#include "pch.h"
#include "Building/FortVolumeManager.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

bool AFortVolumeManager::IsInSpawningStartup() const
{
    static auto bInSpawningStartup = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bInSpawningStartup")));

    return bInSpawningStartup->GetPropertyValue_InContainer(this);
}
