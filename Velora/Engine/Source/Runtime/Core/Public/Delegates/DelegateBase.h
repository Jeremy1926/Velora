#pragma once

#include "HAL/Platform.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Delegates/IDelegateInstance.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/TypeCompatibleBytes.h"

typedef TAlignedBytes<16, 16> AlignedInlineDelegateType;
typedef FHeapAllocator DelegateAllocatorType;

class FDelegateBase
{
public:
    explicit FDelegateBase()
        : DelegateSize(0)
    {
    }

    FDelegateBase(FDelegateBase&& Other)
    {
        DelegateAllocator.MoveToEmpty(Other.DelegateAllocator);
        DelegateSize = Other.DelegateSize;
        Other.DelegateSize = 0;
    }

    ~FDelegateBase()
    {
        Unbind();
    }

    void Unbind()
    {
        if (auto Instance = GetDelegateInstanceProtected())
        {
            IDelegateInstance::Destroy(Instance);
            DelegateAllocator.ResizeAllocation(0, 0, sizeof(AlignedInlineDelegateType));
            DelegateSize = 0;
        }
    }

protected:
    void* GetDelegateInstanceProtected() const
    {
        return DelegateSize ? DelegateAllocator.GetAllocation() : nullptr;
    }

private:
    template<typename ObjectPtrType>
    friend class FMulticastDelegateBase;

    friend void* operator new(size_t Size, FDelegateBase& Base);

    void* Allocate(int32 Size)
    {
        if (auto CurrentInstance = GetDelegateInstanceProtected())
            IDelegateInstance::Destroy(CurrentInstance);

        const int32 NewDelegateSize = FMath::DivideAndRoundUp(Size, (int32)sizeof(AlignedInlineDelegateType));

        if (DelegateSize != NewDelegateSize)
        {
            DelegateAllocator.ResizeAllocation(0, NewDelegateSize, sizeof(AlignedInlineDelegateType));
            DelegateSize = NewDelegateSize;
        }

        return DelegateAllocator.GetAllocation();
    }

    DelegateAllocatorType::ForElementType<AlignedInlineDelegateType> DelegateAllocator;
    int32 DelegateSize;
};

inline void* operator new(size_t Size, FDelegateBase& Base)
{
    return Base.Allocate((int32)Size);
}
