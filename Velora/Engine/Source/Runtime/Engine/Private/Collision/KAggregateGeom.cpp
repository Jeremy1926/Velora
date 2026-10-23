#include "pch.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "Containers/Array.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FKAggregateGeom::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.KAggregateGeom"));

    return Struct;
}

int32 FKAggregateGeom::GetElementIndexByName(const FName InName) const
{
    static auto Name = ((UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.KShapeElem")))->FindPropertyByName(FName(TEXT("Name")));

    int32 ElementIndex = 0;

    for (auto ElemsName : { TEXT("SphereElems"), TEXT("BoxElems"), TEXT("SphylElems"), TEXT("ConvexElems"), TEXT("TaperedCapsuleElems") })
    {
        auto ElemsProperty = (UArrayProperty*)StaticStruct()->FindPropertyByName(FName(ElemsName));
        auto& Elems = *ElemsProperty->ContainerPtrToValuePtr<TArray<uint8>>(this);
        const int32 ElemSize = ElemsProperty->GetInner()->GetSize();

        for (int32 Index = 0; Index < Elems.Num(); Index++, ElementIndex++)
        {
            if (Name->GetPropertyValue_InContainer<FName>(Elems.GetData() + Index * ElemSize) == InName)
                return ElementIndex;
        }
    }

    return INDEX_NONE;
}
