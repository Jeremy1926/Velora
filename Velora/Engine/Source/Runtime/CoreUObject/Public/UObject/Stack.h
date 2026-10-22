#pragma once

#include "Misc/OutputDevice.h"
#include "Containers/Array.h"
#include "UObject/UnrealType.h"

struct FOutParmRec;

typedef uint32 CodeSkipSizeType;

typedef TArray<CodeSkipSizeType, TInlineAllocator<8>> FlowStackType;

struct FFrame : public FOutputDevice
{
public:
    void Step(UObject* Context, void* const Result);

    void StepExplicitProperty(void* const Result, UProperty* Property);

    FORCEINLINE void StepCompiledIn(void* const Result)
    {
        if (Code)
        {
            Step(Object, Result);
        }
        else
        {
            auto Property = (UProperty*)PropertyChainForCompiledIn;
            PropertyChainForCompiledIn = FField::StaticClass() ? (UField*)((FField*)PropertyChainForCompiledIn)->Next : PropertyChainForCompiledIn->Next;
            StepExplicitProperty(Result, Property);
        }
    }

    template<typename TNativeType>
    FORCEINLINE TNativeType& StepCompiledInRef(void* const TemporaryBuffer)
    {
        MostRecentPropertyAddress = nullptr;

        StepCompiledIn(TemporaryBuffer);

        return MostRecentPropertyAddress ? *(TNativeType*)MostRecentPropertyAddress : *(TNativeType*)TemporaryBuffer;
    }

    UFunction* Node;
    UObject* Object;
    uint8* Code;
    uint8* Locals;
    UProperty* MostRecentProperty;
    uint8* MostRecentPropertyAddress;
    FlowStackType FlowStack;
    FFrame* PreviousFrame;
    FOutParmRec* OutParms;
    UField* PropertyChainForCompiledIn;
    UFunction* CurrentNativeFunction;
};
