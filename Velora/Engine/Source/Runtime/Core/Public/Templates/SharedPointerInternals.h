#pragma once

#include "HAL/Platform.h"

enum class ESPMode
{
    NotThreadSafe = 0,
    Fast = 0,
    ThreadSafe = 1
};

template<class ObjectType, ESPMode Mode = ESPMode::Fast> class TSharedPtr;

template<class ObjectType, ESPMode Mode = ESPMode::Fast> class TSharedRef;

namespace SharedPointerInternals
{
    class FReferenceControllerBase
    {
    public:
        int32 SharedReferenceCount;
        int32 WeakReferenceCount;

        virtual void DestroyObject() = 0;

        virtual ~FReferenceControllerBase()
        {
        }
    };

    template<ESPMode Mode>
    struct FReferenceControllerOps;

    template<>
    struct FReferenceControllerOps<ESPMode::NotThreadSafe>
    {
        static FORCEINLINE void AddSharedReference(FReferenceControllerBase* ReferenceController)
        {
            ++ReferenceController->SharedReferenceCount;
        }

        static FORCEINLINE void ReleaseSharedReference(FReferenceControllerBase* ReferenceController)
        {
            if (--ReferenceController->SharedReferenceCount == 0)
            {
                ReferenceController->DestroyObject();
                ReleaseWeakReference(ReferenceController);
            }
        }

        static void ReleaseWeakReference(FReferenceControllerBase* ReferenceController)
        {
            if (--ReferenceController->WeakReferenceCount == 0)
                delete ReferenceController;
        }
    };

    template<>
    struct FReferenceControllerOps<ESPMode::ThreadSafe>
    {
        static FORCEINLINE void AddSharedReference(FReferenceControllerBase* ReferenceController)
        {
            _InterlockedIncrement((volatile long*)&ReferenceController->SharedReferenceCount);
        }

        static FORCEINLINE void ReleaseSharedReference(FReferenceControllerBase* ReferenceController)
        {
            if (_InterlockedDecrement((volatile long*)&ReferenceController->SharedReferenceCount) == 0)
            {
                ReferenceController->DestroyObject();
                ReleaseWeakReference(ReferenceController);
            }
        }

        static void ReleaseWeakReference(FReferenceControllerBase* ReferenceController)
        {
            if (_InterlockedDecrement((volatile long*)&ReferenceController->WeakReferenceCount) == 0)
                delete ReferenceController;
        }
    };

    template<ESPMode Mode>
    class FSharedReferencer
    {
        typedef FReferenceControllerOps<Mode> TOps;

    public:
        FORCEINLINE FSharedReferencer()
            : ReferenceController(nullptr)
        {
        }

        FORCEINLINE explicit FSharedReferencer(FReferenceControllerBase* InReferenceController)
            : ReferenceController(InReferenceController)
        {
        }

        FORCEINLINE FSharedReferencer(FSharedReferencer const& InSharedReference)
            : ReferenceController(InSharedReference.ReferenceController)
        {
            if (ReferenceController != nullptr)
                TOps::AddSharedReference(ReferenceController);
        }

        FORCEINLINE FSharedReferencer(FSharedReferencer&& InSharedReference)
            : ReferenceController(InSharedReference.ReferenceController)
        {
            InSharedReference.ReferenceController = nullptr;
        }

        FORCEINLINE ~FSharedReferencer()
        {
            if (ReferenceController != nullptr)
                TOps::ReleaseSharedReference(ReferenceController);
        }

        inline FSharedReferencer& operator=(FSharedReferencer const& InSharedReference)
        {
            auto NewReferenceController = InSharedReference.ReferenceController;

            if (NewReferenceController != ReferenceController)
            {
                if (NewReferenceController != nullptr)
                    TOps::AddSharedReference(NewReferenceController);

                if (ReferenceController != nullptr)
                    TOps::ReleaseSharedReference(ReferenceController);

                ReferenceController = NewReferenceController;
            }

            return *this;
        }

        inline FSharedReferencer& operator=(FSharedReferencer&& InSharedReference)
        {
            auto NewReferenceController = InSharedReference.ReferenceController;
            auto OldReferenceController = ReferenceController;

            if (NewReferenceController != OldReferenceController)
            {
                InSharedReference.ReferenceController = nullptr;
                ReferenceController = NewReferenceController;

                if (OldReferenceController != nullptr)
                    TOps::ReleaseSharedReference(OldReferenceController);
            }

            return *this;
        }

    private:
        FReferenceControllerBase* ReferenceController;
    };
}
