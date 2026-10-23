#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"

class AActor;
class UScriptStruct;
struct FScalableFloat;

struct FFindActorByClassData
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    const FScalableFloat& GetPrice() const;

    const FScalableFloat& GetRarity() const;
};

class UFortConversationTaskNode_FindActorByClass : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    TArray<AActor*> GetFoundActors(const FConversationContext& Context, FFindActorByClassData& OutFilterData) const;

    void GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const;
};
