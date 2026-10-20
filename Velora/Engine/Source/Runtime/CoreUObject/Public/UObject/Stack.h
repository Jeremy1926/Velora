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
            auto& Version = FEngineVersion::Current();
            auto Property = (UProperty*)PropertyChainForCompiledIn;
            PropertyChainForCompiledIn = Version.GetMajor() > 4 || Version.GetMinor() >= 25 ? (UField*)((FField*)PropertyChainForCompiledIn)->Next : PropertyChainForCompiledIn->Next;
            StepExplicitProperty(Result, Property);
        }
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
