#include "pch.h"
#include "MilkshakeEarthquakeActor.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AMilkshakeEarthquakeActor::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/MilkshakeRuntime.MilkshakeEarthquakeActor"));

    return Class;
}

void AMilkshakeEarthquakeActor::RemoveNullsFromAsyncOverlappedActors()
{
    auto& AsyncOverlappedActors = GetAsyncOverlappedActors();

    for (int32 Index = AsyncOverlappedActors.Num() - 1; Index >= 0; Index--)
    {
        if (!AsyncOverlappedActors[Index])
            AsyncOverlappedActors.RemoveAt(Index);
    }
}

void AMilkshakeEarthquakeActor::execRemoveNullsFromAsyncOverlappedActors(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AMilkshakeEarthquakeActor, execRemoveNullsFromAsyncOverlappedActors, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/MilkshakeRuntime.MilkshakeEarthquakeActor:RemoveNullsFromAsyncOverlappedActors"))
        .Optional()
        .InExec(TEXT("/Script/MilkshakeRuntime.MilkshakeEarthquakeActor:RemoveNullsFromAsyncOverlappedActors")));

    Stack.Code += !!Stack.Code;

    RemoveNullsFromAsyncOverlappedActors();
}

TArray<AActor*>& AMilkshakeEarthquakeActor::GetAsyncOverlappedActors()
{
    static auto AsyncOverlappedActors = GetClass()->FindPropertyByName(FName(TEXT("AsyncOverlappedActors")));

    return *AsyncOverlappedActors->ContainerPtrToValuePtr<TArray<AActor*>>(this);
}
