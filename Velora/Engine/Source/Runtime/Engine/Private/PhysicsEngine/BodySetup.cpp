#include "pch.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UBodySetup::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.BodySetup"));

    return Class;
}

FKAggregateGeom& UBodySetup::GetAggGeom()
{
    static auto AggGeom = StaticClass()->FindPropertyByName(FName(TEXT("AggGeom")));

    return *AggGeom->ContainerPtrToValuePtr<FKAggregateGeom>(this);
}
