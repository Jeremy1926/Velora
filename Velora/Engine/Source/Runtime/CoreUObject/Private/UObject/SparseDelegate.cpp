#include "pch.h"
#include "UObject/SparseDelegate.h"
#include "Scanner.h"

bool FSparseDelegateStorage::Add(const UObject* DelegateOwner, const FName DelegateName, FScriptDelegate Delegate)
{
    static auto Add = Anchor::Function<bool (*)(const UObject*, FName, FScriptDelegate*)>(TEXT("FSparseDelegateStorage::Add"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActor"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("call 0x*; or [*+0x*], al")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.Actor:SetOwner"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/GameplayAbilities.Default__GameplayCueNotify_Actor"))
        .Find("mov r9, [rsp+0x*]; *; *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Find("call 0x*; or [*], al")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.Actor:SetOwner"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/GameplayAbilities.Default__GameplayCueNotify_Actor"))
        .Find("mov r9d, *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov [rbp-0x*], r9d; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*; or [*], al")
        .Target()
        .Get();

    return Add(DelegateOwner, DelegateName, &Delegate);
}

bool FSparseDelegateStorage::AddUnique(const UObject* DelegateOwner, const FName DelegateName, FScriptDelegate Delegate)
{
    static auto AddUnique = Anchor::Function<bool (*)(const UObject*, FName, FScriptDelegate*)>(TEXT("FSparseDelegateStorage::AddUnique"))
        .String(TEXT("OnEnterOverlap"))
        .Find("mov [*+0x*], eax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    return AddUnique(DelegateOwner, DelegateName, &Delegate);
}

bool FSparseDelegateStorage::Remove(const UObject* DelegateOwner, const FName DelegateName, const UObject* ObjectToRemove, const FName FunctionNameToRemove)
{
    static auto Remove = Anchor::Function<bool (*)(const UObject*, FName, const UObject*, FName)>(TEXT("FSparseDelegateStorage::Remove"))
        .Exec(TEXT("/Script/Engine.Actor:SetOwner"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/GameplayAbilities.Default__GameplayCueNotify_Actor"))
        .Find("call 0x*")
        .Target()
        .Find("mov r8d, *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Get();

    return Remove(DelegateOwner, DelegateName, ObjectToRemove, FunctionNameToRemove);
}

bool FSparseDelegateStorage::RemoveAll(const UObject* DelegateOwner, const FName DelegateName, const UObject* UserObject)
{
    static auto RemoveAll = Anchor::Function<bool (*)(const UObject*, FName, const UObject*)>(TEXT("FSparseDelegateStorage::RemoveAll"))
        .String(TEXT("&UClamberingComponent::HandleTargetActorDestroyed"))
        .Start()
        .Find("add rcx, 0x*; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Or()
        .Pattern("48 89 5C 24 08 48 89 6C 24 10 56 57 41 54 41 56 41 57 48 83 EC 30 4C 8B F1 4D 8B E0 48 8D 0D ? ? ? ? 8B DA 48 89 4C 24 20 40 32 ED FF 15 ? ? ? ? 8B 05 ? ? ? ? 4C 8D 3D ? ? ? ? 3B 05 ? ? ? ? 48 8B 35 ? ? ? ? 74 ? 49 8B CE 33 D2 48 C1 E9 04 E8")
        .Get();

    return RemoveAll(DelegateOwner, DelegateName, UserObject);
}
