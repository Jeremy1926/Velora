#include "pch.h"
#include "Pawns/FortPawnComponent_Convert.h"
#include "Pawns/FortPawn.h"
#include "Player/FortPlayerState.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"

UClass* UFortPawnComponent_Convert::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPawnComponent_Convert"));

    return Class;
}

void UFortPawnComponent_Convert::ConvertPawn(AFortPawn* PawnToConvert)
{
    static auto bCanBeConverted = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bCanBeConverted")));
    static auto ConverterPlayerState = StaticClass()->FindPropertyByName(FName(TEXT("ConverterPlayerState")));
    static auto ConverterPlayerInfo = StaticClass()->FindPropertyByName(FName(TEXT("ConverterPlayerInfo")));
    static auto ConverterPlayerInfoState = ConverterPlayerInfo ? ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ConverterPlayerInfo")))->FindPropertyByName(FName(TEXT("ConverterPlayerState"))) : nullptr;

    if (!PawnToConvert)
        return;

    auto ConvertedPawnConvertComponent = PawnToConvert->FindComponentByClass<UFortPawnComponent_Convert>();

    if (!ConvertedPawnConvertComponent || !bCanBeConverted->GetPropertyValue_InContainer(ConvertedPawnConvertComponent))
        return;

    auto PlayerState = ConverterPlayerState ? *ConverterPlayerState->ContainerPtrToValuePtr<AFortPlayerState*>(ConvertedPawnConvertComponent) : ConverterPlayerInfoState->ContainerPtrToValuePtr<TWeakObjectPtr<AFortPlayerState>>(ConverterPlayerInfo->ContainerPtrToValuePtr<void>(ConvertedPawnConvertComponent))->Get();

    if (PlayerState && PlayerState->GetCurrentPawn() && PlayerState->GetCurrentPawn() == GetOwner())
        return;

    InternalConvertPawn(PawnToConvert, ConvertedPawnConvertComponent);
}

void UFortPawnComponent_Convert::InternalConvertPawn(AFortPawn* PawnToConvert, UFortPawnComponent_Convert* ConvertedPawnConvertComponent)
{
    static auto InternalConvertPawn = Anchor::Function<void (*)(UFortPawnComponent_Convert*, AFortPawn*, UFortPawnComponent_Convert*)>(TEXT("UFortPawnComponent_Convert::InternalConvertPawn"))
        .String(TEXT("&UFortPawnComponent_Convert::OnConvertedPawnDied"))
        .Start()
        .Get();

    InternalConvertPawn(this, PawnToConvert, ConvertedPawnConvertComponent);
}
