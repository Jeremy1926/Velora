#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"
#include "UObject/WeakObjectPtr.h"

class FField;
class UStruct;

struct FFieldPath
{
protected:
    mutable FField* ResolvedField;
    mutable TWeakObjectPtr<UStruct> ResolvedOwner;
    TArray<FName> Path;
};

template<class PropertyType>
struct TFieldPath : public FFieldPath
{
};
