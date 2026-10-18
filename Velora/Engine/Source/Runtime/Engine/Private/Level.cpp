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
    static auto bIsAssociatingLevel = Scanner::String(TEXT("sorting actor list")).Start().Find("lea *, [rdx+0x*]");
    static auto Offset = bIsAssociatingLevel.Disp();
    static auto Mask = bIsAssociatingLevel.Find("or byte ptr [*], 0x*").Imm();

    return *((uint8_t*)this + Offset) & Mask;
}

bool ULevel::HasVisibilityChangeRequestPending() const
{
    return (GetOwningWorld() && (this == GetOwningWorld()->GetCurrentLevelPendingVisibility() || this == GetOwningWorld()->GetCurrentLevelPendingInvisibility()));
}
