#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"

class UProperty;
struct FFrame;

typedef void (UObject::*Native)(FFrame& TheStack, void* const Z_Param__Result);

class UField : public UObject
{
public:
    UField* Next;
};

class UStruct : public UField
{
public:
    UProperty* FindPropertyByName(FName InName) const;
};

class UClass : public UStruct
{
};

class UFunction : public UStruct
{
public:
    Native GetNativeFunc() const;
};
