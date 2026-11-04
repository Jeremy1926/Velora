#include "pch.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

bool UKismetSystemLibrary::LineTraceSingle(const UObject* WorldContextObject, const FVector Start, const FVector End, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
    static auto LineTraceSingle = Anchor::Function<bool (*)(const UObject*, const TGameValue<FVector>, const TGameValue<FVector>, ETraceTypeQuery, bool, const TArray<AActor*>&, EDrawDebugTrace::Type, FHitResult&, bool, FLinearColor, FLinearColor, float)>(TEXT("UKismetSystemLibrary::LineTraceSingle"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:LineTraceSingle"))
        .Get();

    return LineTraceSingle(WorldContextObject, Start, End, TraceChannel, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHit, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);
}

bool UKismetSystemLibrary::SphereTraceMultiForObjects(const UObject* WorldContextObject, const FVector Start, const FVector End, float Radius, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, TArray<FHitResult>& OutHits, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
    static auto SphereTraceMultiForObjects = Anchor::Function<bool (*)(const UObject*, const TGameValue<FVector>, const TGameValue<FVector>, float, const TArray<TEnumAsByte<EObjectTypeQuery>>&, bool, const TArray<AActor*>&, EDrawDebugTrace::Type, TArray<FHitResult>&, bool, FLinearColor, FLinearColor, float)>(TEXT("UKismetSystemLibrary::SphereTraceMultiForObjects"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:SphereTraceMultiForObjects"))
        .Get();

    return SphereTraceMultiForObjects(WorldContextObject, Start, End, Radius, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHits, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);
}

bool UKismetSystemLibrary::CapsuleTraceMultiForObjects(const UObject* WorldContextObject, const FVector Start, const FVector End, float Radius, float HalfHeight, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, TArray<FHitResult>& OutHits, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
    static auto CapsuleTraceMultiForObjects = Anchor::Function<bool (*)(const UObject*, const TGameValue<FVector>, const TGameValue<FVector>, float, float, const TArray<TEnumAsByte<EObjectTypeQuery>>&, bool, const TArray<AActor*>&, EDrawDebugTrace::Type, TArray<FHitResult>&, bool, FLinearColor, FLinearColor, float)>(TEXT("UKismetSystemLibrary::CapsuleTraceMultiForObjects"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:CapsuleTraceMultiForObjects"))
        .Get();

    return CapsuleTraceMultiForObjects(WorldContextObject, Start, End, Radius, HalfHeight, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHits, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);
}

bool UKismetSystemLibrary::SphereOverlapActors(const UObject* WorldContextObject, const FVector SpherePos, float SphereRadius, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, UClass* ActorClassFilter, const TArray<AActor*>& ActorsToIgnore, TArray<AActor*>& OutActors)
{
    static auto SphereOverlapActors = Anchor::Function<bool (*)(const UObject*, const TGameValue<FVector>, float, const TArray<TEnumAsByte<EObjectTypeQuery>>&, UClass*, const TArray<AActor*>&, TArray<AActor*>&)>(TEXT("UKismetSystemLibrary::SphereOverlapActors"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:SphereOverlapActors"))
        .Get();

    return SphereOverlapActors(WorldContextObject, SpherePos, SphereRadius, ObjectTypes, ActorClassFilter, ActorsToIgnore, OutActors);
}

FSoftObjectPtr UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId)
{
    static auto GetSoftObjectReferenceFromPrimaryAssetId = Anchor::Function<FSoftObjectPtr (*)(FPrimaryAssetId)>(TEXT("UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:GetSoftObjectReferenceFromPrimaryAssetId"))
        .Get();

    if (FName::IsOutlineNumber())
        return ((FSoftObjectPtr (*)(FPrimaryAssetId::FOutlineNumberLayout))GetSoftObjectReferenceFromPrimaryAssetId)(PrimaryAssetId.ToOutlineNumberLayout());

    return GetSoftObjectReferenceFromPrimaryAssetId(PrimaryAssetId);
}

UObject* UKismetSystemLibrary::GetObjectFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId)
{
    static auto GetObjectFromPrimaryAssetId = Anchor::Function<UObject* (*)(FPrimaryAssetId)>(TEXT("UKismetSystemLibrary::GetObjectFromPrimaryAssetId"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:GetObjectFromPrimaryAssetId"))
        .Get();

    if (FName::IsOutlineNumber())
        return ((UObject* (*)(FPrimaryAssetId::FOutlineNumberLayout))GetObjectFromPrimaryAssetId)(PrimaryAssetId.ToOutlineNumberLayout());

    return GetObjectFromPrimaryAssetId(PrimaryAssetId);
}

FString UKismetSystemLibrary::GetPathName(const UObject* Object)
{
    static auto bInlined = Anchor::Exists()
        .If(!::Anchor::Exists().Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetPathName")).Find("cmp *, 0x1; sbb *, 0xffffffffffffffff; mov [*+0x20], *").Get())
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetPathName"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto ObjectGetPathName = Anchor::Function<FString* (*)(const UObject*, FString*, const UObject*)>(TEXT("UObjectBaseUtility::GetPathName"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetPathName"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("xor r8d, r8d; lea rdx, [*]; call 0x*")
            .Find("call 0x*")
            .Target()
            .Or()
            .Pattern("48 8B DA 33 FF 48 89 3A 49 8B C0 48 89 7A 08 4C 8D 44 24 20 48 8D 54 24 40")
            .Start()
            .Or()
            .Pattern("48 89 5C 24 20 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 90 FE FF FF 48 81 EC 70 02 00 00 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 60 01 00 00 4C 8D 6A 08 48 8B FA 48 8D 5A 0C 45 33 F6 4C 89 32 49 8B C0 48 8D 54 24 60 45 89 75 00 48 89 54 24 40 4C 8D 44 24 40 48 8D 54 24 60 44 89 33 48 89 54 24 48 48 8D 95 60 01 00 00 48 89 54 24 50 48 8B D0 44 88 74 24 58 E8")
            .Get();

        if (!Object)
            return FString(TEXT("None"));

        FString PathName;

        ObjectGetPathName(Object, &PathName, nullptr);

        return PathName;
    }

    static auto GetPathName = Anchor::Function<FString (*)(const UObject*)>(TEXT("UKismetSystemLibrary::GetPathName"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:GetPathName"))
        .Get();

    return GetPathName(Object);
}
