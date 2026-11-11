#include "pch.h"
#include "UObject/UObjectArray.h"
#include "Scanner.h"

FUObjectArray& GUObjectArray = *(FUObjectArray*)(Anchor::Data<uint8_t*>(TEXT("GUObjectArray.ObjObjects"))
    .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
    .Last("cmp *, [0x*]")
    .Find("mov *, [0x*]")
    .Target()
    .Or()
    .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
    .Start()
    .Find("cmp *, [0x*]; jnl *")
    .Find("mov *, [0x*]")
    .Target()
    .Get() - offsetof(FUObjectArray, ObjObjects));

bool FUObjectItem::IsPendingKill() const
{
    static auto RefCountedBit = Anchor::Offset(TEXT("EInternalObjectFlags::RefCounted bit"))
        .Pattern("F0 FF 41 14 8B 41 08 0F BA E0 ?")
        .Find("bt eax, 0x*")
        .Optional()
        .Imm();

    return !!(Flags & int32(EInternalObjectFlags::PendingKill) & ~(RefCountedBit ? 1 << RefCountedBit : 0));
}
