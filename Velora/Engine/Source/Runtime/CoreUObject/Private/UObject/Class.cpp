#include "pch.h"
#include "UObject/Class.h"
#include "Containers/UnrealString.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

const UStruct::FEncryptedLayout* UStruct::GetEncryptedLayout()
{
    if (!FUObjectArray::GetEncryptedLayout())
        return nullptr;

    static FEncryptedLayout Layout
    {
        Anchor::Cipher(TEXT("UStruct::SuperStruct"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:DoesClassImplementInterface"))
            .Find("add [*+0x20], *")
            .Find("call 0x*; mov *l, al")
            .Target()
            .Find("mov r*, [r*+0x*]; test r*, r*; jnz *")
            .Or()
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:DoesClassImplementInterface"))
            .Find("add [*+0x20], *")
            .Find("mov r*, [r*+0x*]; test r*, r*; jnz *")
            .Get(),
        Anchor::Cipher(TEXT("UStruct::PropertiesSize"))
            .Exec(TEXT("/Script/Engine.DataTableFunctionLibrary:GetDataTableRowFromName"))
            .Find("add [*+0x20], *")
            .Find("mov r9d, 0x1")
            .Find("call 0x*")
            .Target()
            .Find("movsx e*, word ptr [r*+0x*]")
            .Start()
            .Find("mov *, [rcx+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.DataTableFunctionLibrary:GetDataTableRowFromName"))
            .Find("add [*+0x20], *")
            .Find("mov r9d, r*; mov r8, rax; mov rdx, r*; call 0x*")
            .Find("call 0x*")
            .Target()
            .Find("movsx e*, word ptr [r*+0x*]")
            .Start()
            .Find("mov *, [rcx+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.DataTableFunctionLibrary:GetDataTableRowFromName"))
            .Find("add [*+0x20], *")
            .Find("mov r9d, 0x1")
            .Find("call 0x*")
            .Target()
            .Find("mov ecx, [rcx+0x*]")
            .Find("mov e*, [rdi+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("UStruct::MinAlignment"))
            .Exec(TEXT("/Script/Engine.DataTableFunctionLibrary:GetDataTableRowFromName"))
            .Find("add [*+0x20], *")
            .Find("mov r9d, 0x1")
            .Find("call 0x*")
            .Target()
            .Find("movsx e*, word ptr [r*+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.DataTableFunctionLibrary:GetDataTableRowFromName"))
            .Find("add [*+0x20], *")
            .Find("mov r9d, r*; mov r8, rax; mov rdx, r*; call 0x*")
            .Find("call 0x*")
            .Target()
            .Find("movsx e*, word ptr [r*+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.DataTableFunctionLibrary:GetDataTableRowFromName"))
            .Find("add [*+0x20], *")
            .Find("mov r9d, 0x1")
            .Find("call 0x*")
            .Target()
            .Find("mov ecx, [rcx+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("UStruct::PropertyLink"))
            .Exec(TEXT("/Script/Engine.DataTableFunctionLibrary:GetDataTableColumnAsString"))
            .Find("add [*+0x20], *")
            .Find("call [r*+0x*]")
            .Last("call 0x*")
            .Target()
            .Call()
            .Find("mov r*, [rcx+0x*]")
            .Get()
    };

    return &Layout;
}

UProperty* UStruct::FindPropertyByName(FName InName) const
{
    auto& Version = FEngineVersion::Current();

    static auto FindPropertyByName = Anchor::Function<UProperty* (*)(const UStruct*, FName)>(TEXT("UStruct::FindPropertyByName"))
        .If(FField::StaticClass())
        .Pattern("48 8B 49 70 48 85 C9 74 ? 4C 8B CA 49 C1 E9 20 48 8B 41 28")
        .Or()
        .If(FField::StaticClass())
        .Pattern("48 8B 41 70 48 85 C0 74 ? 0F 1F 80 00 00 00 00 48 39 50 28 74 ? 48 8B 40 58 48 85 C0 75 ? 33 C0 C3")
        .Or()
        .If(FField::StaticClass())
        .Pattern("48 8B 41 70 EB ? 48 39 50 28 74 ? 48 8B 40 58 48 85 C0 75 ? C3")
        .Or()
        .If(FField::StaticClass())
        .Pattern("48 8B 41 70 48 85 C0 74 ? 39 50 28 74 ? 48 8B 40 50 EB ? C3")
        .Or()
        .If(FField::StaticClass())
        .Pattern("48 8B 41 70 48 85 C0 74 ? 48 39 50 28 74 ? 48 8B 40 58 EB ? C3")
        .Or()
        .If(!FField::StaticClass() && Version.GetMajor() <= 4 && Version.GetMinor() >= 22)
        .Pattern("48 8B 41 68 48 89 54 24 10 48 85 C0 74 ? 44 8B 54 24 14")
        .Or()
        .If(Version.GetMajor() <= 4 && Version.GetMinor() < 22)
        .Pattern("48 8B 41 58 48 85 C0 74 ? 0F 1F 80 00 00 00 00 48 39 50 18 74 ? 48 8B 40 50 48 85 C0 75 ? 33 C0 C3")
        .Or()
        .If(FField::StaticClass())
        .Pattern("48 8B 41 70 48 85 C0 74 ? 39 50 20 74 ? 48 8B 40 40 EB ? C3")
        .Or()
        .If(GetEncryptedLayout() != nullptr)
        .Exec(TEXT("/Script/Engine.DataTableFunctionLibrary:GetDataTableColumnAsString"))
        .Find("add [*+0x20], *")
        .Find("call [r*+0x*]")
        .Last("call 0x*")
        .Target()
        .Call()
        .Get();

    return FindPropertyByName(this, InName);
}

void UStruct::InitializeStruct(void* Dest, int32 ArrayDim) const
{
    static auto InitializeStruct = Anchor::VirtualFunction<void (*)(const UStruct*, void*, int32)>(TEXT("UStruct::InitializeStruct"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:FilterTargetData"))
        .Find("mov r9, [rcx+0x*]; mov r8d, 0x1")
        .Or()
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:FilterTargetData"))
        .Find("mov r8d, 0x1")
        .Find("call [*+0x*]")
        .Get();

    InitializeStruct(this, Dest, ArrayDim);
}

void UStruct::DestroyStruct(void* Dest, int32 ArrayDim) const
{
    static auto DestroyStruct = Anchor::VirtualFunction<void (*)(const UStruct*, void*, int32)>(TEXT("UStruct::InitializeStruct"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:FilterTargetData"))
        .Find("mov r9, [rcx+0x*]; mov r8d, 0x1")
        .Or()
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:FilterTargetData"))
        .Find("mov r8d, 0x1")
        .Find("call [*+0x*]")
        .Get(sizeof(void*));

    DestroyStruct(this, Dest, ArrayDim);
}

void UScriptStruct::CopyScriptStruct(void* Dest, void const* Src, int32 ArrayDim) const
{
    static auto CopyScriptStruct = Anchor::Function<void (*)(const UScriptStruct*, void*, void const*, int32)>(TEXT("UScriptStruct::CopyScriptStruct"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:FilterTargetData"))
        .Find("mov r9d, 0x1")
        .Find("call 0x*")
        .Target()
        .Get();

    CopyScriptStruct(this, Dest, Src, ArrayDim);
}

Native UFunction::GetNativeFunc() const
{
    static auto Func = Anchor::Offset(TEXT("UFunction::Func"))
        .String(TEXT("Failed to bind native function %s.%s"))
        .Start()
        .Find("mov [*+0x*], rax; lea r11, [rsp+0x*]")
        .Or()
        .String(TEXT("Failed to bind native function %s.%s"))
        .Start()
        .Find("mov [*+0x*], rax")
        .Disp();

    return *(Native*)((uint8_t*)this + Func);
}

void UFunction::SetNativeFunc(Native InFunc)
{
    static auto Func = Anchor::Offset(TEXT("UFunction::Func"))
        .String(TEXT("Failed to bind native function %s.%s"))
        .Start()
        .Find("mov [*+0x*], rax; lea r11, [rsp+0x*]")
        .Or()
        .String(TEXT("Failed to bind native function %s.%s"))
        .Start()
        .Find("mov [*+0x*], rax")
        .Disp();

    *(Native*)((uint8_t*)this + Func) = InFunc;
}

EFunctionFlags UFunction::GetFunctionFlags() const
{
    static auto FunctionFlags = Anchor::Offset(TEXT("UFunction::FunctionFlags"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov eax, [r*+0x*]; test al, al")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov edx, [r*+0x*]; mov ecx, edx; and ecx, 0x80")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("test byte ptr [r*+0x*], 0x80")
        .Disp();

    return *(const EFunctionFlags*)((const uint8_t*)this + FunctionFlags);
}

int64 UEnum::GetValueByName(FName InName) const
{
    for (auto& Kvp : Names)
    {
        if (TGameLayout<FName>::Load(&Kvp.Key) == InName)
            return Kvp.Value;
    }

    return -1;
}

FString UEnum::GetNameStringByValue(int64 InValue) const
{
    static auto GetNameStringByValue = Anchor::Function<FString* (*)(const UEnum*, FString*, int64)>(TEXT("UEnum::GetNameStringByValue"))
        .String(TEXT("Action Router - Player [%d]: Input Type[%s]"))
        .Last("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Action Router - Player [%d]: Input Type[%s]"))
        .Start()
        .CallSite()
        .Last("call 0x*")
        .Target()
        .Get();

    FString Result;
    GetNameStringByValue(this, &Result, InValue);

    return Result;
}

UObject* UClass::GetDefaultObject(bool bCreateIfNeeded) const
{
    static auto ClassDefaultObject = Anchor::Offset(TEXT("UClass::ClassDefaultObject"))
        .String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'."))
        .Start()
        .Find("call [rax+0x*]; mov *, [*+0x*]")
        .Find("mov *, [*+0x*]")
        .Or()
        .String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'."))
        .Start()
        .Find("mov dl, 0x1; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov *, [*+0x*]")
        .Or()
        .String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'."))
        .Start()
        .Find("mov dl, 0x1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov *, [*+0x*]")
        .Or()
        .String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'."))
        .Start()
        .Find("mov dl, r15b; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov *, [*+0x*]")
        .Disp();

    if (*(UObject**)((uint8_t*)this + ClassDefaultObject) == nullptr && bCreateIfNeeded)
    {
        InternalCreateDefaultObjectWrapper();
    }

    return *(UObject**)((uint8_t*)this + ClassDefaultObject);
}

UObject* UClass::InternalCreateDefaultObjectWrapper() const
{
    static auto CreateDefaultObject = Anchor::VirtualFunction<UObject* (*)(const UClass*)>(TEXT("UClass::CreateDefaultObject"))
        .String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'."))
        .Start()
        .Find("call [rax+0x*]; mov *, [*+0x*]")
        .Or()
        .String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'."))
        .Start()
        .Find("mov dl, 0x1; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]")
        .Or()
        .String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'."))
        .Start()
        .Find("mov dl, 0x1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]")
        .Or()
        .String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'."))
        .Start()
        .Find("mov dl, 0x1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("jmp [rax+0x*]")
        .Or()
        .String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'."))
        .Start()
        .Find("mov dl, r15b; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]")
        .Get();

    return CreateDefaultObject(this);
}

UClass* UClass::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CoreUObject.Class"));

    return Class;
}

UFunction* UClass::FindFunctionByName(FName InName) const
{
    for (auto Struct = (const UStruct*)this; Struct; Struct = Struct->GetSuperStruct())
    {
        if (auto Function = (UFunction*)StaticFindObject(nullptr, (UObject*)Struct, *InName.ToString()))
            return Function;
    }

    return nullptr;
}
