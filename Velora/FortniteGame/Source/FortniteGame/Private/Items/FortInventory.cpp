#include "pch.h"
#include "Items/FortInventory.h"
#include "Weapons/FortWeaponModItemDefinition.h"
#include "GameplayTagContainer.h"
#include "Misc/EngineVersion.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UClass* AFortInventory::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventory"));

    return Class;
}

EFortInventoryType& AFortInventory::GetInventoryType()
{
    static auto InventoryType = GetClass()->FindPropertyByName(FName(TEXT("InventoryType")));

    return *InventoryType->ContainerPtrToValuePtr<EFortInventoryType>(this);
}

FFortItemList& AFortInventory::GetInventory()
{
    static auto Inventory = GetClass()->FindPropertyByName(FName(TEXT("Inventory")));

    return *Inventory->ContainerPtrToValuePtr<FFortItemList>(this);
}

void AFortInventory::HandleInventoryLocalUpdate()
{
    static auto InterfaceHandleInventoryLocalUpdate = Anchor::VirtualFunction<void (*)(void*)>(TEXT("IFortInventoryInterface::HandleInventoryLocalUpdate"))
        .Exec(TEXT("/Script/FortniteGame.FortInventory:HandleInventoryLocalUpdate"))
        .Find("add rcx, 0x*; add [rdx+0x20], rax; mov rax, [rcx]; jmp [rax+0x*]")
        .Find("jmp [rax+0x*]")
        .Optional()
        .Get();

    if (InterfaceHandleInventoryLocalUpdate)
    {
        static auto FortInventoryInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryInterface"));
        static auto InterfaceMarkArrayDirty = Anchor::VirtualFunction<void (*)(void*)>(TEXT("IFortInventoryInterface::MarkArrayDirty"))
            .Virtual((const void* const*)Scanner::FindVTable(TEXT("/Script/FortniteGame.Default__FortInventory"), TEXT("/Script/FortniteGame.FortInventoryInterface")), [](const Anchor::FCode& Entry) { return Entry.Is("push rbx; sub rsp, 0x20; mov rbx, rcx; add rcx, 0x*; call 0x*; xor eax, eax; mov [rbx+0x*], rax; mov [rbx+0x*], rax; mov [rbx+0x*], rax"); })
            .Optional()
            .Get();

        if (auto Interface = GetInterfaceAddress(FortInventoryInterface))
        {
            if (InterfaceMarkArrayDirty)
                InterfaceMarkArrayDirty(Interface);

            InterfaceHandleInventoryLocalUpdate(Interface);
        }

        return;
    }

    static auto HandleInventoryLocalUpdate = Anchor::Function<void (*)(AFortInventory*)>(TEXT("AFortInventory::HandleInventoryLocalUpdate"))
        .Exec(TEXT("/Script/FortniteGame.FortInventory:HandleInventoryLocalUpdate"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    HandleInventoryLocalUpdate(this);
}

int32 AFortInventory::GetOverflowFromAddingItem(const FFortItemEntry& ItemEntry) const
{
    static auto GetOverflowFromAddingItemForOwner = Anchor::Optional<int32 (*)(const FFortItemEntry&, void*, uint8, bool)>(TEXT("AFortInventory::GetOverflowFromAddingItem"))
        .String(TEXT("SwapEquippedText"))
        .Find("mov rdx, rax; call 0x*; test eax, eax")
        .Call()
        .Get();

    if (GetOverflowFromAddingItemForOwner)
    {
        static auto FortInventoryOwnerInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryOwnerInterface"));
        static auto InventoryType = GetClass()->FindPropertyByName(FName(TEXT("InventoryType")));
        auto Owner = GetOwner();

        return GetOverflowFromAddingItemForOwner(ItemEntry, Owner ? Owner->GetInterfaceAddress(FortInventoryOwnerInterface) : nullptr, *InventoryType->ContainerPtrToValuePtr<uint8>(this), false);
    }

    static auto InterfaceGetOverflowFromAddingItem = Anchor::VirtualFunction<int32 (*)(const void*, const FFortItemEntry&)>(TEXT("IFortInventoryInterface::GetOverflowFromAddingItem"))
        .String(TEXT("SwapEquippedText"))
        .Find("mov r8, [rcx+0x*]; mov rcx, r*; call r8; test eax, eax")
        .Optional()
        .Get();

    if (InterfaceGetOverflowFromAddingItem)
    {
        static auto FortInventoryInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryInterface"));
        auto Interface = const_cast<AFortInventory*>(this)->GetInterfaceAddress(FortInventoryInterface);

        return Interface ? InterfaceGetOverflowFromAddingItem(Interface, ItemEntry) : 0;
    }

    static auto GetOverflowFromAddingItem = Anchor::Function<int32 (*)(const AFortInventory*, const FFortItemEntry&)>(TEXT("AFortInventory::GetOverflowFromAddingItem"))
        .String(TEXT("SwapEquippedText"))
        .Find("mov rcx, *; movzx *, al; call 0x*; test eax, eax")
        .Call()
        .Or()
        .String(TEXT("SwapEquippedText"))
        .FindEither("mov rcx, rax; call 0x*", "mov rcx, *; call 0x*; test eax, eax")
        .Call()
        .Has("lea rcx, [rsp+0x*]; call 0x*; mov rdx, *; lea rcx, [rsp+0x*]; call 0x*")
        .Or()
        .String(TEXT("SwapEquippedText"))
        .Find("cmp [rsp+0x*], r*; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("call 0x*; test eax, eax")
        .Call()
        .Has("xor r9d, r9d; mov r8d, *; mov rdx, rax; mov rcx, *")
        .Or()
        .String(TEXT("SwapEquippedText"))
        .Find("mov r*, [rsp+0x*]; test r*, r*; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("call 0x*; test eax, eax")
        .Call()
        .Has("xor r9d, r9d; mov r8d, *; mov rdx, rax; mov rcx, *")
        .Or()
        .If(FEngineVersion::Current().GetMajor() > 4)
        .String(TEXT("SwapEquippedText"))
        .Find("mov rcx, r*; call 0x*; test eax, eax; jnz 0x*")
        .Call()
        .Has("xor r9d, r9d; mov r8d, *; mov rdx, rax; mov rcx, *")
        .Or()
        .String(TEXT("SwapEquippedText"))
        .Find("mov rcx, r*; mov r*, al; call 0x*; test eax, eax; jnz 0x*")
        .Call()
        .Has("xor r9d, r9d; mov r8d, *; mov rdx, rax; mov rcx, *")
        .Get();

    return GetOverflowFromAddingItem(this, ItemEntry);
}

void AFortInventory::SetItemRequiresUpdate(FFortItemEntry& ItemEntry)
{
    static auto InterfaceSetItemRequiresUpdate = Anchor::VirtualFunction<void (*)(void*, FFortItemEntry&)>(TEXT("IFortInventoryInterface::SetItemRequiresUpdate"))
        .String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"))
        .Find("mov edx, *; call 0x*")
        .Call()
        .Find("mov [*+0xc], e*; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [r8+0x*]")
        .Or()
        .String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"))
        .Start()
        .Find("mov edx, *; call 0x*")
        .Call()
        .Find("mov dl, 0x1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [r8+0x*]")
        .Or()
        .String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"))
        .Start()
        .Find("mov edx, *; call 0x*")
        .Call()
        .Find("cmp rcx, [rax]; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("mov dl, 0x1; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("test dl, dl; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("call [r8+0x*]")
        .Or()
        .VirtualStartHas(Scanner::FindVTable(TEXT("/Script/FortniteGame.Default__FortInventory"), TEXT("/Script/FortniteGame.FortInventoryInterface")), "cmp byte ptr [rcx-0x*], 0x3; mov rdi, rdx; mov rbx, rcx")
        .Optional()
        .Get();

    if (InterfaceSetItemRequiresUpdate)
    {
        static auto FortInventoryInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryInterface"));

        if (auto Interface = GetInterfaceAddress(FortInventoryInterface))
            InterfaceSetItemRequiresUpdate(Interface, ItemEntry);

        return;
    }

    static auto SetItemRequiresUpdate = Anchor::Function<void (*)(AFortInventory*, FFortItemEntry&)>(TEXT("AFortInventory::SetItemRequiresUpdate"))
        .String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"))
        .Find("mov rcx, rax; mov rdx, *; call 0x*")
        .Call()
        .Or()
        .String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"))
        .Find("mov edx, ebp; call 0x*")
        .Call()
        .Find("mov rcx, rax; mov rdx, *; call 0x*")
        .Call()
        .Or()
        .String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"))
        .Find("mov edx, *; call 0x*")
        .Call()
        .Call()
        .Find("mov rdx, rbx; mov rcx, r9; call 0x*")
        .Call()
        .Or()
        .String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"))
        .Find("mov edx, *; call 0x*")
        .Call()
        .Call()
        .Find("mov rdx, rbx; mov rcx, r8; call 0x*")
        .Call()
        .Or()
        .String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"))
        .Find("mov edx, *; call 0x*")
        .Call()
        .Call()
        .Find("mov rdx, rbx; mov rcx, rax; call 0x*")
        .Call()
        .Or()
        .Pattern("48 89 5C 24 10 57 48 83 EC 40 80 B9 ? ? 00 00 03 48 8B FA 48 8B D9 0F 85 ? ? ? ? 80 BA ? ? 00 00 00 48 89 74 24 50")
        .Get();

    SetItemRequiresUpdate(this, ItemEntry);
}

FBoolPropertyReference::FBoolPropertyReference(const UBoolProperty* InProperty, void* InContainer)
    : Property(InProperty)
    , Container(InContainer)
{
}

FBoolPropertyReference::operator bool() const
{
    return Property->GetPropertyValue_InContainer(Container);
}

void FBoolPropertyReference::operator=(bool NewValue)
{
    Property->SetPropertyValue_InContainer(Container, NewValue);
}

FFortItemEntryStateValue::FFortItemEntryStateValue(EFortItemEntryState::Type InStateType, int32 InIntValue)
{
    memset(this, 0, GetTypeSize());

    GetIntValue() = InIntValue;
    GetStateType() = InStateType;
}

UScriptStruct* FFortItemEntryStateValue::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemEntryStateValue"));

    return Struct;
}

int32 FFortItemEntryStateValue::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

int32& FFortItemEntryStateValue::GetIntValue()
{
    static auto IntValue = StaticStruct()->FindPropertyByName(FName(TEXT("IntValue")));

    return *IntValue->ContainerPtrToValuePtr<int32>(this);
}

TEnumAsByte<EFortItemEntryState::Type>& FFortItemEntryStateValue::GetStateType()
{
    static auto StateType = StaticStruct()->FindPropertyByName(FName(TEXT("StateType")));

    return *StateType->ContainerPtrToValuePtr<TEnumAsByte<EFortItemEntryState::Type>>(this);
}

FFortItemEntryStateValueConstObject::FFortItemEntryStateValueConstObject(const FGameplayTag& InStateType, const UObject* InObjectData)
{
    static auto StateType = StaticStruct()->FindPropertyByName(FName(TEXT("StateType")));

    memset(this, 0, GetTypeSize());

    StateType->SetPropertyValue_InContainer<FGameplayTag>(this, InStateType);
    GetObjectData() = (UObject*)InObjectData;
}

UScriptStruct* FFortItemEntryStateValueConstObject::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemEntryStateValueConstObject"));

    return Struct;
}

int32 FFortItemEntryStateValueConstObject::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

FGameplayTag FFortItemEntryStateValueConstObject::GetStateType() const
{
    static auto StateType = StaticStruct()->FindPropertyByName(FName(TEXT("StateType")));

    return StateType->GetPropertyValue_InContainer<FGameplayTag>(this);
}

TWeakObjectPtr<UObject>& FFortItemEntryStateValueConstObject::GetObjectData()
{
    static auto ObjectData = StaticStruct()->FindPropertyByName(FName(TEXT("ObjectData")));

    return *ObjectData->ContainerPtrToValuePtr<TWeakObjectPtr<UObject>>(this);
}

FFortItemEntry::FFortItemEntry(UFortItemDefinition* InItemDefinition, int32 InCount, int32 InLevel)
{
    static auto VariantGuidConstructor = Anchor::Optional<void (*)(FFortItemEntry*, UFortItemDefinition*, int32, int32, const FGuid&)>(TEXT("FFortItemEntry::FFortItemEntry(ItemVariantGuid)"))
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; movups [*], xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Has("mov rax, [rsp+0x50]; movups xmm0, [rax]")
        .Or()
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; movups [*], xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Has("mov rax, [rsp+0x60]; movups xmm0, [rax]")
        .Or()
        .Pattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 20 33 ED 48 89 51 18 48 8D B1 ? ? 00 00 48 C7 01 FF FF FF FF")
        .Has("mov rax, [rsp+0x50]; movups xmm0, [rax]")
        .Get();

    if (VariantGuidConstructor)
    {
        VariantGuidConstructor(this, InItemDefinition, InCount, InLevel, FGuid());

        return;
    }

    static auto Constructor = Anchor::Function<void (*)(FFortItemEntry*, UFortItemDefinition*, int32, int32, const TArray<UObject*>*)>(TEXT("FFortItemEntry::FFortItemEntry"))
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    Constructor(this, InItemDefinition, InCount, InLevel, nullptr);
}

FFortItemEntry::FFortItemEntry(const FFortItemEntry& Other)
{
    static auto Constructor = Anchor::Optional<void (*)(FFortItemEntry*, const FFortItemEntry&)>(TEXT("FFortItemEntry::FFortItemEntry(const FFortItemEntry&)"))
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("lea r8d, [r9+0x1]; call 0x*; mov rdx, rax")
        .Call(2)
        .Get();

    if (Constructor)
    {
        Constructor(this, Other);

        return;
    }

    StaticStruct()->InitializeStruct(this);
    StaticStruct()->CopyScriptStruct(this, &Other);
}

FFortItemEntry::~FFortItemEntry()
{
    static auto Destructor = Anchor::Function<void (*)(FFortItemEntry*)>(TEXT("FFortItemEntry::~FFortItemEntry"))
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; call 0x*")
        .Find("add rcx, [*]")
        .Find("lea rcx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; movups [*], xmm0; call 0x*")
        .Find("add rcx, [*]")
        .Find("lea rcx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("add rcx, [*]")
        .Find("lea rcx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 C7 81 88 01 00 00 00 00 00 00 48 8B D9 48 8B B1 80 01 00 00 48 85 F6 74 ? 48 8B 0D ? ? ? ? 48 85 C9 75 ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 48 8B D6 FF 50 48 48 8D 8B 40 01 00 00 E8")
        .Get();

    Destructor(this);
}

UScriptStruct* FFortItemEntry::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemEntry"));

    return Struct;
}

int32 FFortItemEntry::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

int32& FFortItemEntry::GetCount()
{
    static auto Count = StaticStruct()->FindPropertyByName(FName(TEXT("Count")));

    return *Count->ContainerPtrToValuePtr<int32>(this);
}

UFortItemDefinition*& FFortItemEntry::GetItemDefinition()
{
    static auto ItemDefinition = StaticStruct()->FindPropertyByName(FName(TEXT("ItemDefinition")));

    return *ItemDefinition->ContainerPtrToValuePtr<UFortItemDefinition*>(this);
}

float& FFortItemEntry::GetDurability()
{
    static auto Durability = StaticStruct()->FindPropertyByName(FName(TEXT("Durability")));

    return *Durability->ContainerPtrToValuePtr<float>(this);
}

int32& FFortItemEntry::GetLevel()
{
    static auto Level = StaticStruct()->FindPropertyByName(FName(TEXT("Level")));

    return *Level->ContainerPtrToValuePtr<int32>(this);
}

int32& FFortItemEntry::GetLoadedAmmo()
{
    static auto LoadedAmmo = StaticStruct()->FindPropertyByName(FName(TEXT("LoadedAmmo")));

    return *LoadedAmmo->ContainerPtrToValuePtr<int32>(this);
}

FGuid& FFortItemEntry::GetItemGuid()
{
    static auto ItemGuidProperty = StaticStruct()->FindPropertyByName(FName(TEXT("ItemGuid")));

    auto& ItemGuid = *ItemGuidProperty->ContainerPtrToValuePtr<FGuid>(this);

    if (!ItemGuid.IsValid())
        ItemGuid = FGuid::NewGuid();

    return ItemGuid;
}

FBoolPropertyReference FFortItemEntry::GetIsReplicatedCopy()
{
    static auto bIsReplicatedCopy = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bIsReplicatedCopy")));

    return FBoolPropertyReference(bIsReplicatedCopy, this);
}

FBoolPropertyReference FFortItemEntry::GetIsDirty()
{
    static auto bIsDirty = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bIsDirty")));

    return FBoolPropertyReference(bIsDirty, this);
}

TArray<FFortItemEntryStateValue>& FFortItemEntry::GetStateValues()
{
    static auto StateValues = StaticStruct()->FindPropertyByName(FName(TEXT("StateValues")));

    return *StateValues->ContainerPtrToValuePtr<TArray<FFortItemEntryStateValue>>(this);
}

TWeakObjectPtr<AFortInventory>& FFortItemEntry::GetParentInventory()
{
    static auto ParentInventory = StaticStruct()->FindPropertyByName(FName(TEXT("ParentInventory")));

    return *ParentInventory->ContainerPtrToValuePtr<TWeakObjectPtr<AFortInventory>>(this);
}

void FFortItemEntry::SetCount(int32 NewCount)
{
    NewCount = FMath::Max(NewCount, 0);

    if (GetCount() != NewCount)
    {
        GetCount() = NewCount;

        if (GetParentInventory().IsValid())
        {
            GetParentInventory().Get()->SetItemRequiresUpdate(*this);
        }

        GetIsDirty() = true;
    }
}

void FFortItemEntry::SetDurability(float NewDurability)
{
    if (GetDurability() != NewDurability)
    {
        GetDurability() = NewDurability;

        if (GetParentInventory().IsValid())
        {
            GetParentInventory().Get()->SetItemRequiresUpdate(*this);
        }

        GetIsDirty() = true;
    }
}

void FFortItemEntry::SetLoadedAmmo(int32 NewLoadedAmmo)
{
    if (GetLoadedAmmo() != NewLoadedAmmo)
    {
        GetLoadedAmmo() = NewLoadedAmmo;

        if (GetParentInventory().IsValid())
        {
            GetParentInventory().Get()->SetItemRequiresUpdate(*this);
        }

        GetIsDirty() = true;
    }
}

bool FFortItemEntry::GetStateValue(EFortItemEntryState::Type StateType, int32& OutIntValue)
{
    for (auto& StateValue : GetStateValues())
    {
        if (StateValue.GetStateType() == StateType)
        {
            OutIntValue = StateValue.GetIntValue();
            return true;
        }
    }

    return false;
}

void FFortItemEntry::SetStateValue(EFortItemEntryState::Type StateType, int32 IntValue)
{
    auto& StateValues = GetStateValues();
    int32 StateValueIndex = 0;

    while (StateValueIndex < StateValues.Num() && StateValues[StateValueIndex].GetStateType() != StateType)
    {
        StateValueIndex++;
    }

    if (StateValueIndex < StateValues.Num())
    {
        StateValues[StateValueIndex].GetIntValue() = IntValue;
    }
    else
    {
        new(StateValues) FFortItemEntryStateValue(StateType, IntValue);
    }

    if (GetParentInventory().IsValid())
    {
        GetParentInventory().Get()->SetItemRequiresUpdate(*this);
    }

    GetIsDirty() = true;
}

void FFortItemEntry::RemoveStateValue(EFortItemEntryState::Type StateType)
{
    auto& StateValues = GetStateValues();

    for (int32 StateValueIndex = 0; StateValueIndex < StateValues.Num(); StateValueIndex++)
    {
        if (StateValues[StateValueIndex].GetStateType() == StateType)
        {
            StateValues.RemoveAt(StateValueIndex);

            if (GetParentInventory().IsValid())
            {
                GetParentInventory().Get()->SetItemRequiresUpdate(*this);
            }

            GetIsDirty() = true;

            return;
        }
    }
}

TArray<FFortItemEntryStateValueConstObject>& FFortItemEntry::GetStateValuesConstObject()
{
    static auto StateValuesConstObject = StaticStruct()->FindPropertyByName(FName(TEXT("StateValuesConstObject")));

    return *StateValuesConstObject->ContainerPtrToValuePtr<TArray<FFortItemEntryStateValueConstObject>>(this);
}

const UObject* FFortItemEntry::GetStateConstObject(const FGameplayTag& StateType)
{
    for (auto& StateValue : GetStateValuesConstObject())
    {
        if (StateValue.GetStateType() == StateType)
            return StateValue.GetObjectData().Get();
    }

    return nullptr;
}

void FFortItemEntry::SetStateConstObject(const FGameplayTag& StateType, const UObject* ObjectData)
{
    auto& StateValues = GetStateValuesConstObject();
    int32 StateValueIndex = 0;

    while (StateValueIndex < StateValues.Num() && !(StateValues[StateValueIndex].GetStateType() == StateType))
    {
        StateValueIndex++;
    }

    if (StateValueIndex < StateValues.Num())
    {
        if (StateValues[StateValueIndex].GetObjectData().Get() == ObjectData)
            return;

        StateValues[StateValueIndex].GetObjectData() = (UObject*)ObjectData;
    }
    else
    {
        new(StateValues) FFortItemEntryStateValueConstObject(StateType, ObjectData);
    }

    if (GetParentInventory().IsValid())
    {
        GetParentInventory().Get()->SetItemRequiresUpdate(*this);
    }

    GetIsDirty() = true;
}

void FFortItemEntry::RemoveStateConstObject(const FGameplayTag& StateType)
{
    auto& StateValues = GetStateValuesConstObject();

    for (int32 StateValueIndex = 0; StateValueIndex < StateValues.Num(); StateValueIndex++)
    {
        if (StateValues[StateValueIndex].GetStateType() == StateType)
        {
            StateValues.RemoveAt(StateValueIndex);

            if (GetParentInventory().IsValid())
            {
                GetParentInventory().Get()->SetItemRequiresUpdate(*this);
            }

            GetIsDirty() = true;

            return;
        }
    }
}

TArray<FFortWeaponModSlot>& FFortItemEntry::GetWeaponModSlots()
{
    static auto WeaponModSlots = StaticStruct()->FindPropertyByName(FName(TEXT("WeaponModSlots")));

    return *WeaponModSlots->ContainerPtrToValuePtr<TArray<FFortWeaponModSlot>>(this);
}

TArray<FFortSavedWeaponModSlot>& FFortItemEntry::GetSavedWeaponModSlots()
{
    static auto SavedWeaponModSlots = StaticStruct()->FindPropertyByName(FName(TEXT("SavedWeaponModSlots")));

    return *SavedWeaponModSlots->ContainerPtrToValuePtr<TArray<FFortSavedWeaponModSlot>>(this);
}

void FFortItemEntry::AppendWeaponModSlot(const FFortWeaponModSlot& WeaponModSlot)
{
    GetWeaponModSlots().Add(WeaponModSlot);
    GetSavedWeaponModSlots().Add(FFortSavedWeaponModSlot(WeaponModSlot));

    if (GetParentInventory().IsValid())
    {
        GetParentInventory().Get()->SetItemRequiresUpdate(*this);
    }

    GetIsDirty() = true;
}

void FFortItemEntry::SetWeaponModSlots(const TArray<FFortWeaponModSlot>& InWeaponModSlots)
{
    GetWeaponModSlots() = InWeaponModSlots;
    GetSavedWeaponModSlots().Reset(InWeaponModSlots.Num());

    for (auto& WeaponModSlot : InWeaponModSlots)
    {
        GetSavedWeaponModSlots().Add(FFortSavedWeaponModSlot(WeaponModSlot));
    }

    if (GetParentInventory().IsValid())
    {
        GetParentInventory().Get()->SetItemRequiresUpdate(*this);
    }

    GetIsDirty() = true;
}

void FFortItemEntry::CopyWeaponModStateFrom(FFortItemEntry& Other)
{
    static auto WeaponModSlots = StaticStruct()->FindPropertyByName(FName(TEXT("WeaponModSlots")));
    static auto StateValuesConstObject = StaticStruct()->FindPropertyByName(FName(TEXT("StateValuesConstObject")));

    if (WeaponModSlots && Other.GetWeaponModSlots().Num() > 0)
        SetWeaponModSlots(Other.GetWeaponModSlots());

    if (StateValuesConstObject)
    {
        for (auto& StateValue : Other.GetStateValuesConstObject())
            SetStateConstObject(StateValue.GetStateType(), StateValue.GetObjectData().Get());
    }
}

UScriptStruct* FFortItemList::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemList"));

    return Struct;
}

TArray<FFortItemEntry>& FFortItemList::GetReplicatedEntries()
{
    static auto ReplicatedEntries = StaticStruct()->FindPropertyByName(FName(TEXT("ReplicatedEntries")));

    return *ReplicatedEntries->ContainerPtrToValuePtr<TArray<FFortItemEntry>>(this);
}

TArray<UFortWorldItem*>& FFortItemList::GetItemInstances()
{
    static auto ItemInstances = StaticStruct()->FindPropertyByName(FName(TEXT("ItemInstances")));

    return *ItemInstances->ContainerPtrToValuePtr<TArray<UFortWorldItem*>>(this);
}
