#include "pch.h"
#include "SimObject.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

TMulticastDelegate<void, AActor*>& ASimObject::GetNotifyNetCullDistanceChange()
{
    static auto NotifyNetCullDistanceChange = Anchor::Data<TMulticastDelegate<void, AActor*>*>(TEXT("ASimObject::NotifyNetCullDistanceChange"))
        .FieldPattern("0F 2E 89 %o 74 ? F3 0F 11 89 %o 48 8B D1 48 8D 0D", AActor::StaticClass() ? AActor::StaticClass()->FindPropertyByName(FName(TEXT("NetCullDistanceSquared"))) : nullptr)
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyNetCullDistanceChange;
}
