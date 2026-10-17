#include "pch.h"
#include "Building/BuildingActor.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

void ABuildingActor::BeginPlay()
{
    static auto BeginPlay = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("ReceiveBeginPlay"));
        const uint8_t* ReceiveBeginPlayName = nullptr;

        for (auto Address = Reference; !ReceiveBeginPlayName;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Address != Reference && Instruction.opcode == 0x8D && (Instruction.modrm & 0xC7) == 0x05)
                ReceiveBeginPlayName = Address + Length + (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        const uint8_t* ReceiveBeginPlay = nullptr;

        for (auto& Section : Scanner::Sections())
        {
            if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                continue;

            auto Begin = Scanner::Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin; Address + 7 <= End && !ReceiveBeginPlay; Address++)
            {
                if (Address[0] == 0x48 && Address[1] == 0x8B && Address[2] == 0x15 && Address + 7 + *(int32_t*)(Address + 3) == ReceiveBeginPlayName)
                    ReceiveBeginPlay = Scanner::Base() + Scanner::FindFunction(Address)->BeginAddress;
            }
        }

        auto ActorBeginPlay = Scanner::Base() + Scanner::FindFunction(Scanner::FindCallReference(ReceiveBeginPlay))->BeginAddress;
        auto ActorVTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto Index = 0;

        while (ActorVTable[Index] != ActorBeginPlay)
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingActor"));

    (*(void (**)(ABuildingActor*))(VTable + BeginPlay))(this);
}

void ABuildingActor::SilentDie()
{
    static auto SilentDie = FindFunctionChecked(FName(TEXT("SilentDie")));

    ProcessEvent(SilentDie, nullptr);
}

float ABuildingActor::GetMaxHealth() const
{
    static auto GetMaxHealth = FindFunctionChecked(FName(TEXT("GetMaxHealth")));

    struct
    {
        float ReturnValue;
    } Parms{};

    const_cast<ABuildingActor*>(this)->ProcessEvent(GetMaxHealth, &Parms);

    return Parms.ReturnValue;
}

bool ABuildingActor::HasHealthLeft() const
{
    static auto HasHealthLeft = FindFunctionChecked(FName(TEXT("HasHealthLeft")));

    struct
    {
        bool ReturnValue;
    } Parms{};

    const_cast<ABuildingActor*>(this)->ProcessEvent(HasHealthLeft, &Parms);

    return Parms.ReturnValue;
}

const FGameplayTagContainer& ABuildingActor::GetStaticGameplayTags() const
{
    static auto StaticGameplayTags = GetClass()->FindPropertyByName(FName(TEXT("StaticGameplayTags")));

    return *StaticGameplayTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this);
}

bool ABuildingActor::GetDestroyed() const
{
    static auto bDestroyed = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDestroyed")));

    return bDestroyed->GetPropertyValue_InContainer(this);
}
