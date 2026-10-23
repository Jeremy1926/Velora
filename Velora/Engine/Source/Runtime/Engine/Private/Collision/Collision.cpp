#include "pch.h"
#include "Engine/EngineTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

FHitResult::FHitResult()
{
    FMemory::Memzero(this, GetTypeSize());
    GetTime() = 1.f;
}

UScriptStruct* FHitResult::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.HitResult"));

    return Struct;
}

int32 FHitResult::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

float& FHitResult::GetTime()
{
    static auto Time = StaticStruct()->FindPropertyByName(FName(TEXT("Time")));

    return *Time->ContainerPtrToValuePtr<float>(this);
}

FVector_NetQuantize& FHitResult::GetLocation()
{
    static auto Location = StaticStruct()->FindPropertyByName(FName(TEXT("Location")));

    return *Location->ContainerPtrToValuePtr<FVector_NetQuantize>(this);
}

TWeakObjectPtr<AActor>& FHitResult::GetActor()
{
    static auto Actor = StaticStruct()->FindPropertyByName(FName(TEXT("Actor")));
    static auto HitObjectHandle = StaticStruct()->FindPropertyByName(FName(TEXT("HitObjectHandle")));

    return *(Actor ? Actor : HitObjectHandle)->ContainerPtrToValuePtr<TWeakObjectPtr<AActor>>(this);
}

const TWeakObjectPtr<UPrimitiveComponent>& FHitResult::GetComponent() const
{
    static auto Component = StaticStruct()->FindPropertyByName(FName(TEXT("Component")));

    return *Component->ContainerPtrToValuePtr<TWeakObjectPtr<UPrimitiveComponent>>(this);
}

uint8 FHitResult::GetElementIndex() const
{
    static auto ElementIndex = StaticStruct()->FindPropertyByName(FName(TEXT("ElementIndex")));

    return *ElementIndex->ContainerPtrToValuePtr<uint8>(this);
}
