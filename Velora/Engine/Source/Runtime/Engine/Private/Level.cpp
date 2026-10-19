#include "pch.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UWorld* ULevel::GetOwningWorld() const
{
    static auto OwningWorld = GetClass()->FindPropertyByName(FName(TEXT("OwningWorld")));

    return *OwningWorld->ContainerPtrToValuePtr<UWorld*>(this);
}

bool ULevel::IsAssociatingLevel() const
{
    static auto AddToWorld = Scanner::String(TEXT("sorting actor list")).Start();
    static auto bIsAssociatingLevel = AddToWorld.Find("lea *, [rdx+0x*]");
    static auto SetAssociating = bIsAssociatingLevel.Get() ? bIsAssociatingLevel.Find("or byte ptr [*], 0x*") : AddToWorld.Find("or byte ptr [rdx+0x*], 0x*");
    static auto Offset = (bIsAssociatingLevel.Get() ? bIsAssociatingLevel : SetAssociating).Disp(TEXT("ULevel::bIsAssociatingLevel"));
    static auto Mask = SetAssociating.Imm();

    return *((uint8_t*)this + Offset) & Mask;
}

bool ULevel::HasVisibilityChangeRequestPending() const
{
    return (GetOwningWorld() && (this == GetOwningWorld()->GetCurrentLevelPendingVisibility() || this == GetOwningWorld()->GetCurrentLevelPendingInvisibility()));
}
