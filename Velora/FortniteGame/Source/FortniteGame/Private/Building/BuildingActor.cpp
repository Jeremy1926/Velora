#include "pch.h"
#include "Building/BuildingActor.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void ABuildingActor::BeginPlay()
{
    static auto BeginPlay = []
    {
        auto ReceiveBeginPlayName = Scanner::String(TEXT("ReceiveBeginPlay")).Find("lea *, [0x*]").Target();
        auto ReceiveBeginPlay = Scanner::Reference(ReceiveBeginPlayName, "mov rdx, *").Start().Get();
        auto ActorBeginPlay = Scanner::CallTo(ReceiveBeginPlay).Start().Get();
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
