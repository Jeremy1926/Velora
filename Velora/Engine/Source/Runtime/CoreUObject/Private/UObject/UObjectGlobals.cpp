#include "pch.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void FObjectInitializer::AssertIfInConstructor(UObject* Outer, const TCHAR* ErrorMessage)
{
    static auto AssertIfInConstructor = Anchor::Function<void (*)(UObject*, const TCHAR*)>(TEXT("FObjectInitializer::AssertIfInConstructor"))
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."))
        .FirstOf("call 0x*", "jmp 0x*")
        .Is("call 0x*")
        .Call()
        .Or()
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."), "lea rdx, [0x*]; call 0x*")
        .Call()
        .Or()
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSubobject<> instead."))
        .Start()
        .Get();

    AssertIfInConstructor(Outer, ErrorMessage);
}

UObject* GetTransientPackage()
{
    static auto GObjTransientPkg = Anchor::Data<UObject**>(TEXT("GObjTransientPkg"))
        .String(TEXT("/Engine/Transient"))
        .Find("mov [0x*], rax")
        .Target()
        .Get();

    return *GObjTransientPkg;
}

UObject* StaticFindObject(UClass* ObjectClass, UObject* InObjectPackage, const TCHAR* OrigInName, bool ExactClass)
{
    static auto StaticFindObject = Anchor::Function<UObject* (*)(UClass*, UObject*, const TCHAR*, bool)>(TEXT("StaticFindObject"))
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"), "lea rax, [0x*]; mov r9d, 0x1; mov edx, 0x*; mov [rsp+0x20], rax; call 0x*; lea r9, [0x*]; mov r8d, 0x*; lea rdx, [0x*]; lea rcx, [0x*]; call 0x*; cmp *, 0xffffffffffffffff")
        .Start()
        .Or()
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"), "lea rsi, [0x*]; mov r9d, 0x1; mov [rbp+0x*], rax; lea r8, [rbp+0x*]; mov [rsp+0x20], rsi; mov edx, 0x*; lea rcx, [0x*]; call 0x*; mov r9, rsi; lea rdx, [0x*]; mov r8d, 0x*; lea rcx, [0x*]; call 0x*; cmp *, 0xffffffffffffffff")
        .Start()
        .Or()
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"), "lea rax, [0x*]; mov r9d, 0x1; lea r8, [0x*]; mov [rsp+0x20], rax; mov edx, 0x*; lea rcx, [0x*]; call 0x*; call 0x*; cmp *, 0xffffffffffffffff")
        .Start()
        .Or()
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"))
        .Start()
        .Or()
        .Pattern("45 33 F6 4C 8B E1 45 0F B6 E9 49 8B F8 41 8B C6 41 8B CE 48 83 FA FF 0F 84")
        .Start()
        .Or()
        .String(TEXT("GlobalUMGSequenceTickManager"), "lea r8, [0x*]; mov rdx, rbx; mov rcx, rax; add rsp, 0x*; pop rbx; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Or()
        .String(TEXT("GlobalUMGSequenceTickManager"))
        .Find("call 0x*")
        .Target()
        .Get();

    auto Object = StaticFindObject(ObjectClass, InObjectPackage, OrigInName, ExactClass);

    if (!Object && OrigInName && FString(OrigInName).StartsWith(TEXT("/Script/FortniteGame.")))
    {
        FString MovedName(TEXT("/Script/FortniteAI."));
        MovedName += OrigInName + wcslen(TEXT("/Script/FortniteGame."));

        Object = StaticFindObject(ObjectClass, InObjectPackage, *MovedName, ExactClass);
    }

    return Object;
}

UObject* StaticLoadObject(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename, uint32 LoadFlags, UPackageMap* Sandbox, bool bAllowObjectReconciliation, FUObjectSerializeContext* InSerializeContext)
{
    static auto StaticLoadObject = Anchor::Function<UObject* (*)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32, UPackageMap*, bool, FUObjectSerializeContext*)>(TEXT("StaticLoadObject"))
        .String(TEXT("Failed to find object '{ClassName} {OuterName}.{ObjectName}'"))
        .Start()
        .Get();

    return StaticLoadObject(Class, InOuter, Name, Filename, LoadFlags, Sandbox, bAllowObjectReconciliation, InSerializeContext);
}

UObject* StaticConstructObject_Internal(UClass* Class, UObject* InOuter, FName Name, EObjectFlags SetFlags, EInternalObjectFlags InternalSetFlags, UObject* Template, bool bCopyTransientsFromClassDefaults, FObjectInstancingGraph* InstanceGraph, bool bAssumeTemplateIsArchetype)
{
    static auto StaticConstructObject_InternalWithParams = Anchor::Optional<UObject* (*)(const void*)>(TEXT("StaticConstructObject_Internal(FStaticConstructObjectParameters)"))
        .String(TEXT("STAT_ConstructObject"))
        .Start()
        .Find("mov *, [rcx]; lea rax, [0x*]; mov *, [rcx+0x8]")
        .Start()
        .Or()
        .String(TEXT("STAT_ConstructObject"), "lea rax, [0x*]; mov *, [rcx+0x8]")
        .Start()
        .Or()
        .String(TEXT("STAT_ConstructObject"), "lea *, [0x*]; mov *, [rcx+0x8]")
        .Start()
        .Get();

    if (StaticConstructObject_InternalWithParams)
    {
        auto ConstructWithParams = [&](auto GameName)
        {
            struct
            {
                UClass* Class;
                UObject* Outer;
                decltype(GameName) Name;
                EObjectFlags SetFlags;
                EInternalObjectFlags InternalSetFlags;
                bool bCopyTransientsFromClassDefaults;
                bool bAssumeTemplateIsArchetype;
                UObject* Template;
                FObjectInstancingGraph* InstanceGraph;
                void* ExternalPackage;
                alignas(16) uint8 PropertyInitCallback[0x40];
                void* SubobjectOverrides;
            } Params { Class, InOuter, GameName, SetFlags, InternalSetFlags, bCopyTransientsFromClassDefaults, bAssumeTemplateIsArchetype, Template, InstanceGraph, nullptr };

            return StaticConstructObject_InternalWithParams(&Params);
        };

        if (!FName::IsOutlineNumber())
            return ConstructWithParams(Name);

        NAME_INDEX OutlineName = 0;
        TGameLayout<FName>::Store(&OutlineName, Name);

        return ConstructWithParams(OutlineName);
    }

    static auto StaticConstructObject_Internal = Anchor::Function<UObject* (*)(UClass*, UObject*, FName, EObjectFlags, EInternalObjectFlags, UObject*, bool, FObjectInstancingGraph*, bool)>(TEXT("StaticConstructObject_Internal"))
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."))
        .FirstOf("call 0x*", "jmp 0x*")
        .Is("call 0x*")
        .Call(2)
        .Or()
        .String(TEXT("STAT_ConstructObject"))
        .Start()
        .Or()
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."), "lea rdx, [0x*]; call 0x*")
        .Call(2)
        .Get();

    return StaticConstructObject_Internal(Class, InOuter, Name, SetFlags, InternalSetFlags, Template, bCopyTransientsFromClassDefaults, InstanceGraph, bAssumeTemplateIsArchetype);
}

static Anchor::FCipher HighHalf(Anchor::FCipher Cipher)
{
    Cipher.Offset += Cipher.Size / 2;
    Cipher.Size /= 2;

    return Cipher;
}

const FUObjectArray::FEncryptedLayout* FUObjectArray::GetItemLayout()
{
    static auto AllocateSerialNumber = Anchor::Address()
        .Pattern("F0 0F C1 1D ? ? ? ? FF C3 81 FB E8 03 00 00")
        .Get();

    static auto AllocateSerialNumberMember = Anchor::Address()
        .If(!AllocateSerialNumber)
        .Pattern("B8 01 00 00 00 F0 0F C1 01 8D 70 01 3D E7 03 00 00")
        .Or()
        .If(!AllocateSerialNumber)
        .Pattern("B8 01 00 00 00 F0 0F C1 41 ? 8D 70 01 3D E7 03 00 00")
        .Or()
        .If(!AllocateSerialNumber)
        .Pattern("B8 01 00 00 00 F0 0F C1 81 ? ? ? ? 8D 70 01 3D E7 03 00 00")
        .Get();

    static auto ObjectArray = Anchor::Address()
        .If(AllocateSerialNumberMember != nullptr)
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:DelayUntilNextTick"))
        .Find("lea rcx, [0x*]; mov edx, *; call 0x*")
        .Target()
        .Get();

    if (!AllocateSerialNumber && !ObjectArray)
        return nullptr;

    static auto NumElements = AllocateSerialNumber ? Anchor::At(AllocateSerialNumber).Last("mov e*, [0x*]").Cipher() : Anchor::At(AllocateSerialNumberMember).Start().Find("* e*, [rcx+0x*]").Cipher().Rebased(ObjectArray);
    static auto Objects = AllocateSerialNumber ? Anchor::At(AllocateSerialNumber).Last("mov r*, [0x*]").Cipher() : Anchor::At(AllocateSerialNumberMember).Start().Find("* r*, [rcx+0x*]").Cipher().Rebased(ObjectArray);
    static auto bEncrypted = NumElements.IsEncrypted() && Objects.IsEncrypted();

    static auto PackedItem = Anchor::Address()
        .At(AllocateSerialNumber)
        .Last("lea r*, [r*+rdx?4]; mov e*, [r*+0x*]")
        .Last("lea rdx, [r*+r*?4]")
        .Or()
        .At(AllocateSerialNumber)
        .Last("lea r*, [r*+rdx?4]; jmp *; xor e*, e*; mov e*, [r*+0x*]")
        .Last("lea rdx, [r*+r*?4]")
        .Get();

    if (!bEncrypted && (!PackedItem || NumElements.IsEncrypted() || Objects.IsEncrypted()))
        return nullptr;

    static auto ItemFlags = Anchor::Address()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_SoftObjectReferenceToObject"))
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Find("mov e*, [r*]; test e*, 0x*; jnz *; mov e*, [r*]; mov e*, [r*]")
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_SoftObjectReferenceToObject"))
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Find("mov r*, [r*]; sar r*, 0x20; test e*, 0x*; jnz *; mov *, [r*]; mov *, [r*]")
        .Get();

    static auto PackedPointer = Anchor::At(ItemFlags).Find("or r*, r*; shl r*, 0x3").Get();
    static auto PackedFlags = ItemFlags && PackedPointer && PackedPointer < Anchor::At(ItemFlags).Find("j*", 2).Get() ? ItemFlags : nullptr;

    if (!bEncrypted && !PackedFlags)
        return nullptr;

    static auto Flags = Anchor::At(PackedFlags).Is("mov r*, [r*]; sar r*, 0x20") ? HighHalf(Anchor::At(PackedFlags).Cipher()) : Anchor::At(PackedFlags).Cipher();
    static auto FirstLow = Anchor::At(PackedFlags).Find("mov e*, [r*]", 1).Cipher();
    static auto SecondLow = Anchor::At(PackedFlags).Find("mov e*, [r*]", 2).Cipher();

    static auto MemberFlags = Anchor::Address()
        .If(AllocateSerialNumberMember != nullptr)
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_SoftObjectReferenceToObject"))
        .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("mov r*, [r*]; mov r*, 0x*; test r*, r*; jz 0x*")
        .Or()
        .If(AllocateSerialNumberMember != nullptr)
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_SoftObjectReferenceToObject"))
        .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
        .Find("call 0x*")
        .Target()
        .Find("mov r*, [r*]; mov r*, 0x*; test r*, r*; jnz 0x*")
        .Get();

    static auto MemberLow = Anchor::Address()
        .At(MemberFlags)
        .Is("mov r*, [r*]; mov r*, 0x*; test r*, r*; jz 0x*")
        .Find("jz 0x*")
        .Target()
        .FirstOf("mov *, 0x*; * *, [r*+0x*]", "* *, [r*+0x*]")
        .Or()
        .At(MemberFlags)
        .Find("jnz 0x*")
        .Find("* *, [r*+0x*]")
        .Get();

    static FEncryptedLayout Layout
    {
        NumElements,
        Objects,
        Anchor::At(AllocateSerialNumber ? AllocateSerialNumber : AllocateSerialNumberMember).Last("mov e*, [r*+0x*]").Cipher(TEXT("FUObjectItem::SerialNumber")),
        MemberFlags ? HighHalf(Anchor::At(MemberFlags).Cipher()) : PackedFlags ? Flags : Anchor::Cipher(TEXT("FUObjectItem::Flags"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_SoftObjectReferenceToObject"))
            .Find("add [*+0x20], *")
            .Find("call 0x*")
            .Target()
            .Find("mov e*, [r*+0x*]; test e*, 0x*")
            .Get(),
        MemberLow ? Anchor::At(MemberLow).Cipher() : PackedFlags ? (FirstLow.Offset != Flags.Offset ? FirstLow : SecondLow) : Anchor::Cipher(TEXT("FUObjectItem::Object"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_SoftObjectReferenceToObject"))
            .Find("add [*+0x20], *")
            .Find("call 0x*")
            .Target()
            .Find("mov e*, [r*+0x*]; test e*, 0x*")
            .Find("mov r*, [*]")
            .Get(),
        Anchor::At(AllocateSerialNumber).Last("shl r*, 0x5; add r*, [*]; jmp *; xor e*, e*; mov e*, [r*+0x*]").Get() ? 0x20 : PackedItem ? 0x14 : 0x18,
        PackedFlags != nullptr || MemberLow != nullptr
    };

    return &Layout;
}

const FUObjectArray::FEncryptedLayout* FUObjectArray::GetEncryptedLayout()
{
    auto Layout = GetItemLayout();

    return Layout && Layout->NumElements.IsEncrypted() && Layout->Objects.IsEncrypted() ? Layout : nullptr;
}

const UObjectBase::FEncryptedLayout* UObjectBase::GetEncryptedLayout()
{
    if (!FUObjectArray::GetEncryptedLayout())
        return nullptr;

    static FEncryptedLayout Layout
    {
        Anchor::Cipher(TEXT("UObjectBase::ObjectFlags"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
            .Find("add [*+0x20], *")
            .Find("mov e*, [rax+0x*]; bt e*, 0x1e")
            .Or()
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValidClass"))
            .Find("add [*+0x20], *")
            .Find("mov e*, [rax+0x*]; bt e*, 0x1e")
            .Or()
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
            .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
            .Find("mov e*, [rax+0x*]; bt e*, 0x1e")
            .Or()
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
            .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
            .Find("bt dword ptr [rax+0x*], 0x1e")
            .Or()
            .Exec(TEXT("/Script/Engine.LevelStreaming:CreateInstance"))
            .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
            .Find("test dword ptr [rax+0x*], 0x30; jnz *; mov rax, [rax+0x*]; test rax, rax; jnz *")
            .Get(),
        Anchor::Cipher(TEXT("UObjectBase::InternalIndex"))
            .Pattern("F0 0F C1 1D ? ? ? ? FF C3 81 FB E8 03 00 00")
            .Last("mov r*, [0x*]")
            .Last("mov *, [r*+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:K2_ClearTimer"))
            .Find("lea rcx, [0x*]; mov edx, *; call 0x*")
            .Last("* e*, [r*+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:DelayUntilNextTick"))
            .Find("lea rcx, [0x*]; mov edx, *; call 0x*")
            .Last("* r*, [r*+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("UObjectBase::ClassPrivate"))
            .Exec(TEXT("/Script/Engine.GameplayStatics:GetObjectClass"))
            .Find("add [*+0x20], *")
            .Find("test rax, rax; jz *; mov *, [rax+0x*]")
            .Find("mov *, [rax+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.GameplayStatics:GetObjectClass"))
            .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
            .Find("test rax, rax; jz *; mov *, [rax+0x*]")
            .Find("mov *, [rax+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("UObjectBase::NamePrivate"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetObjectName"))
            .Find("add [*+0x20], *")
            .Find("mov eax, [rax+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetObjectName"))
            .Find("add [*+0x20], *")
            .Find("call 0x*")
            .Target()
            .Find("mov eax, [rdx+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetObjectName"))
            .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
            .Find("mov eax, [rax+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("UObjectBase::OuterPrivate"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetOuterObject"))
            .Find("add [*+0x20], *")
            .Find("test rax, rax; jz *; mov *, [rax+0x*]")
            .Find("mov *, [rax+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetOuterObject"))
            .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
            .Find("test rax, rax; jz *; mov *, [rax+0x*]")
            .Find("mov *, [rax+0x*]")
            .Get()
    };

    return &Layout;
}
