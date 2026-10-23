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
