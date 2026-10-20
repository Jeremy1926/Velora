#include "pch.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

bool UKismetSystemLibrary::LineTraceSingle(const UObject* WorldContextObject, const FVector Start, const FVector End, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
    static auto LineTraceSingle = Anchor::Function<bool (*)(const UObject*, const FVector, const FVector, ETraceTypeQuery, bool, const TArray<AActor*>&, EDrawDebugTrace::Type, FHitResult&, bool, FLinearColor, FLinearColor, float)>(TEXT("UKismetSystemLibrary::LineTraceSingle"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:LineTraceSingle"))
        .Get();

    return LineTraceSingle(WorldContextObject, Start, End, TraceChannel, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHit, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);
}

bool UKismetSystemLibrary::SphereTraceMultiForObjects(const UObject* WorldContextObject, const FVector Start, const FVector End, float Radius, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, TArray<FHitResult>& OutHits, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
    static auto SphereTraceMultiForObjects = Anchor::Function<bool (*)(const UObject*, const FVector, const FVector, float, const TArray<TEnumAsByte<EObjectTypeQuery>>&, bool, const TArray<AActor*>&, EDrawDebugTrace::Type, TArray<FHitResult>&, bool, FLinearColor, FLinearColor, float)>(TEXT("UKismetSystemLibrary::SphereTraceMultiForObjects"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:SphereTraceMultiForObjects"))
        .Get();

    return SphereTraceMultiForObjects(WorldContextObject, Start, End, Radius, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHits, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);
}

bool UKismetSystemLibrary::CapsuleTraceMultiForObjects(const UObject* WorldContextObject, const FVector Start, const FVector End, float Radius, float HalfHeight, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, TArray<FHitResult>& OutHits, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
    static auto CapsuleTraceMultiForObjects = Anchor::Function<bool (*)(const UObject*, const FVector, const FVector, float, float, const TArray<TEnumAsByte<EObjectTypeQuery>>&, bool, const TArray<AActor*>&, EDrawDebugTrace::Type, TArray<FHitResult>&, bool, FLinearColor, FLinearColor, float)>(TEXT("UKismetSystemLibrary::CapsuleTraceMultiForObjects"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:CapsuleTraceMultiForObjects"))
        .Get();

    return CapsuleTraceMultiForObjects(WorldContextObject, Start, End, Radius, HalfHeight, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHits, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);
}

FSoftObjectPtr UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId)
{
    static auto GetSoftObjectReferenceFromPrimaryAssetId = Anchor::Function<FSoftObjectPtr (*)(FPrimaryAssetId)>(TEXT("UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:GetSoftObjectReferenceFromPrimaryAssetId"))
        .Get();

    return GetSoftObjectReferenceFromPrimaryAssetId(PrimaryAssetId);
}
