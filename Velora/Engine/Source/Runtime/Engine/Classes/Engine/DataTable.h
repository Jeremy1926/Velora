#pragma once

#include "UObject/Object.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "Containers/Map.h"

struct FTableRowBase
{
    virtual ~FTableRowBase()
    {
    }
};

class UDataTable : public UObject
{
public:
    template <class T>
    T* FindRow(FName RowName, const TCHAR* ContextString, bool bWarnIfRowMissing = true) const
    {
        if (RowStruct == nullptr)
            return nullptr;

        if (!RowStruct->IsChildOf(T::StaticStruct()))
            return nullptr;

        if (RowName == NAME_None)
            return nullptr;

        uint8* const* RowDataPtr = RowMap.Find(RowName);

        if (RowDataPtr == nullptr)
            return nullptr;

        return reinterpret_cast<T*>(*RowDataPtr);
    }

    UScriptStruct* RowStruct;
    TMap<FName, uint8*> RowMap;
};

struct FDataTableRowHandle
{
    template <class T>
    T* GetRow(const TCHAR* ContextString) const
    {
        if (DataTable == nullptr)
            return nullptr;

        return DataTable->FindRow<T>(RowName, ContextString);
    }

    const UDataTable* DataTable;
    FName RowName;
};

struct FDataTableCategoryHandle
{
    const UDataTable* DataTable;
    FName ColumnName;
    FName RowContents;
};
