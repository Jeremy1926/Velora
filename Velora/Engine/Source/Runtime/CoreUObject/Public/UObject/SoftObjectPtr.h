#pragma once

#include "UObject/AssetPtr.h"

class UObject;

template<class T = UObject>
class TSoftObjectPtr
{
private:
    FSoftObjectPtr SoftObjectPtr;
};

template<class TClass = UObject>
class TSoftClassPtr
{
private:
    FSoftObjectPtr SoftObjectPtr;
};
