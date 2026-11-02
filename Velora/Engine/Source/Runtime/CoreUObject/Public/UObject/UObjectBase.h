#pragma once

#include <Anchor/Anchor.h>
#include "HAL/Platform.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectMacros.h"

class UClass;
class UObject;

class UObjectBase
{
public:
    virtual ~UObjectBase();

    struct FEncryptedLayout
    {
        Anchor::FCipher ObjectFlags;
        Anchor::FCipher InternalIndex;
        Anchor::FCipher ClassPrivate;
        Anchor::FCipher NamePrivate;
        Anchor::FCipher OuterPrivate;
    };

    static const FEncryptedLayout* GetEncryptedLayout();

    FORCEINLINE uint32 GetUniqueID() const
    {
        auto Layout = GetEncryptedLayout();

        return Layout ? Layout->InternalIndex.Load<uint32>(this) : (uint32)InternalIndex;
    }

    FORCEINLINE UClass* GetClass() const
    {
        auto Layout = GetEncryptedLayout();

        return Layout ? Layout->ClassPrivate.Load<UClass*>(this) : ClassPrivate;
    }

    FORCEINLINE UObject* GetOuter() const
    {
        auto Layout = GetEncryptedLayout();

        return Layout ? Layout->OuterPrivate.Load<UObject*>(this) : OuterPrivate;
    }

    FORCEINLINE FName GetFName() const
    {
        auto Layout = GetEncryptedLayout();

        return TGameLayout<FName>::Load(Layout ? (const uint8*)this + Layout->NamePrivate.Offset : (const uint8*)&NamePrivate);
    }

    FORCEINLINE EObjectFlags GetFlags() const
    {
        auto Layout = GetEncryptedLayout();

        return Layout ? Layout->ObjectFlags.Load<EObjectFlags>(this) : ObjectFlags;
    }

private:
    EObjectFlags ObjectFlags;
    int32 InternalIndex;
    UClass* ClassPrivate;
    FName NamePrivate;
    UObject* OuterPrivate;
};
