#include "pch.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

bool UKismetSystemLibrary::SphereTraceMultiForObjects(const UObject* WorldContextObject, const FVector Start, const FVector End, float Radius, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, TArray<FHitResult>& OutHits, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
    static auto SphereTraceMultiForObjects = Scanner::Function<bool (*)(const UObject*, const FVector, const FVector, float, const TArray<TEnumAsByte<EObjectTypeQuery>>&, bool, const TArray<AActor*>&, EDrawDebugTrace::Type, TArray<FHitResult>&, bool, FLinearColor, FLinearColor, float)>(Scanner::Native(TEXT("/Script/Engine.KismetSystemLibrary:SphereTraceMultiForObjects")).Get(TEXT("UKismetSystemLibrary::SphereTraceMultiForObjects")));

    return SphereTraceMultiForObjects(WorldContextObject, Start, End, Radius, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHits, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);
}

bool UKismetSystemLibrary::CapsuleTraceMultiForObjects(const UObject* WorldContextObject, const FVector Start, const FVector End, float Radius, float HalfHeight, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, TArray<FHitResult>& OutHits, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
    static auto CapsuleTraceMultiForObjects = Scanner::Function<bool (*)(const UObject*, const FVector, const FVector, float, float, const TArray<TEnumAsByte<EObjectTypeQuery>>&, bool, const TArray<AActor*>&, EDrawDebugTrace::Type, TArray<FHitResult>&, bool, FLinearColor, FLinearColor, float)>(Scanner::Native(TEXT("/Script/Engine.KismetSystemLibrary:CapsuleTraceMultiForObjects")).Get(TEXT("UKismetSystemLibrary::CapsuleTraceMultiForObjects")));

    return CapsuleTraceMultiForObjects(WorldContextObject, Start, End, Radius, HalfHeight, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHits, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);
}
