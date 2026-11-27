#pragma once

#include "Building/BuildingActor.h"

struct FFrame;

struct FServerOnlyDynamicBuilderClass
{
    const TCHAR* ProjectileClass;
    const TCHAR* BuilderClassProperty;
    const TCHAR* Name;
    const TCHAR* BuildingInstructions;
    bool bSelectiveDestruction;
    bool bDestroyMatchingPiece;
    const TCHAR* DynamicBuildingPlacementType;
};

class AFortDynamicBuilder : public ABuildingActor
{
public:
    static UClass* StaticClass();

    static const FServerOnlyDynamicBuilderClass* FindServerOnlyBuilderClass(const FFrame& Stack, UObject* WorldContextObject);

    void SetServerOnlyBuilderClassDefaults(const FServerOnlyDynamicBuilderClass& BuilderClass);
};
