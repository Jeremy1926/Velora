#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"

class UProperty;

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
