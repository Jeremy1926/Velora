#pragma once

#include "HAL/Platform.h"
#include "HAL/UnrealMemory.h"

struct FScriptContainerElement
{
};

class FHeapAllocator
{
public:
    enum { NeedsElementType = false };

    class ForAnyElementType
    {
    public:
        ForAnyElementType()
            : Data(nullptr)
        {
        }

        FORCEINLINE void MoveToEmpty(ForAnyElementType& Other)
        {
            if (Data)
            {
                FMemory::Free(Data);
            }

            Data = Other.Data;
            Other.Data = nullptr;
        }

        FORCEINLINE ~ForAnyElementType()
        {
            if (Data)
            {
                FMemory::Free(Data);
            }
        }

        FORCEINLINE FScriptContainerElement* GetAllocation() const
        {
            return Data;
        }

    private:
        ForAnyElementType(const ForAnyElementType&);
        ForAnyElementType& operator=(const ForAnyElementType&);

        FScriptContainerElement* Data;
    };

    template<typename ElementType>
    class ForElementType : public ForAnyElementType
    {
    public:
        FORCEINLINE ElementType* GetAllocation() const
        {
            return (ElementType*)ForAnyElementType::GetAllocation();
        }
    };
};

class FDefaultAllocator : public FHeapAllocator
{
public:
    typedef FHeapAllocator Typedef;
};
