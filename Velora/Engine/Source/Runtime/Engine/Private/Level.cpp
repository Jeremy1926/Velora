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
    static auto Offset = Anchor::Offset(TEXT("ULevel::bIsAssociatingLevel"))
        .String(TEXT("sorting actor list"))
        .Start()
        .Find("lea *, [rdx+0x*]")
        .Or()
        .String(TEXT("sorting actor list"))
        .Start()
        .Find("or byte ptr [rdx+0x*], 0x*")
        .Disp();

    static auto Mask = Anchor::Offset(TEXT("ULevel::bIsAssociatingLevel"))
        .String(TEXT("sorting actor list"))
        .Start()
        .Find("lea *, [rdx+0x*]")
        .Find("or byte ptr [*], 0x*")
        .Or()
        .String(TEXT("sorting actor list"))
        .Start()
        .Find("or byte ptr [rdx+0x*], 0x*")
        .Value();

    return *((uint8_t*)this + Offset) & Mask;
}

bool ULevel::HasVisibilityChangeRequestPending() const
{
    return (GetOwningWorld() && (this == GetOwningWorld()->GetCurrentLevelPendingVisibility() || this == GetOwningWorld()->GetCurrentLevelPendingInvisibility()));
}
