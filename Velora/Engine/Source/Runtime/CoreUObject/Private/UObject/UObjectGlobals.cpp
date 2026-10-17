#include "pch.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

static auto NewObjectCalls = []
{
    auto Reference = Scanner::FindStringReference(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."));
    std::array<const uint8_t*, 2> Calls = {};
    auto Count = 0ull;

    for (auto Address = Reference; Count < Calls.size();)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8)
            Calls[Count++] = Address + Length + (int32_t)Instruction.imm.imm32;

        Address += Length;
    }

    return Calls;
}();

void FObjectInitializer::AssertIfInConstructor(UObject* Outer, const TCHAR* ErrorMessage)
{
    static auto AssertIfInConstructor = (void (*)(UObject*, const TCHAR*))NewObjectCalls[0];

    AssertIfInConstructor(Outer, ErrorMessage);
}

UObject* StaticFindObject(UClass* ObjectClass, UObject* InObjectPackage, const TCHAR* OrigInName, bool ExactClass)
{
    static auto StaticFindObject = (UObject* (*)(UClass*, UObject*, const TCHAR*, bool))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Illegal call to StaticFindObject() while collecting garbage!")))->BeginAddress);

    return StaticFindObject(ObjectClass, InObjectPackage, OrigInName, ExactClass);
}

UObject* StaticLoadObject(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename, uint32 LoadFlags, UPackageMap* Sandbox, bool bAllowObjectReconciliation)
{
    static auto StaticLoadObject = (UObject* (*)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32, UPackageMap*, bool))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Failed to find object '{ClassName} {OuterName}.{ObjectName}'")))->BeginAddress);

    return StaticLoadObject(Class, InOuter, Name, Filename, LoadFlags, Sandbox, bAllowObjectReconciliation);
}

UObject* StaticConstructObject_Internal(UClass* Class, UObject* InOuter, FName Name, EObjectFlags SetFlags, EInternalObjectFlags InternalSetFlags, UObject* Template, bool bCopyTransientsFromClassDefaults, FObjectInstancingGraph* InstanceGraph, bool bAssumeTemplateIsArchetype)
{
    static auto StaticConstructObject_Internal = (UObject* (*)(UClass*, UObject*, FName, EObjectFlags, EInternalObjectFlags, UObject*, bool, FObjectInstancingGraph*, bool))NewObjectCalls[1];

    return StaticConstructObject_Internal(Class, InOuter, Name, SetFlags, InternalSetFlags, Template, bCopyTransientsFromClassDefaults, InstanceGraph, bAssumeTemplateIsArchetype);
}
