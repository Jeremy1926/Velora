#include "pch.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UClass* ULevel::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Level"));

    return Class;
}

UWorld* ULevel::GetOwningWorld() const
{
    static auto OwningWorld = GetClass()->FindPropertyByName(FName(TEXT("OwningWorld")));

    return *OwningWorld->ContainerPtrToValuePtr<UWorld*>(this);
}

bool ULevel::IsPersistentLevel() const
{
    bool bIsPersistent = false;

    if (GetOwningWorld())
        bIsPersistent = (this == GetOwningWorld()->GetPersistentLevel());

    return bIsPersistent;
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

bool ULevel::IsNetActor(const AActor* Actor)
{
    if (!Actor)
        return false;

    const auto NetRole = !Actor->IsNetMode(NM_Client) ? Actor->GetRemoteRole() : Actor->GetLocalRole();

    return NetRole != ROLE_None;
}
