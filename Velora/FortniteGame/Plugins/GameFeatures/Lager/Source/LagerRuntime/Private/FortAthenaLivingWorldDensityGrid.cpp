#include "pch.h"
#include "FortAthenaLivingWorldDensityGrid.h"
#include "GameFramework/Actor.h"
#include "Scanner.h"

int32 FFortAthenaLivingWorldDensityGrid::NextActorUpdateIndex = 0;

TArray<FFortAthenaLivingWorldDensityGrid::FActorDensityRange> FFortAthenaLivingWorldDensityGrid::ActorDensityRanges;

bool FFortAthenaLivingWorldDensityGrid::HasActorDensityValues()
{
    static auto bHasActorDensityValues = Anchor::Exists()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Optional()
        .Find("mov rcx, r14; mov rdx, [rdx+0x*]; movaps xmm2, xmm0; call 0x*")
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rsi+0x*]; mov rdx, [rdx+0x*]; movaps xmm2, xmm0; call 0x*")
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [r14+0x*]; mov rdx, [rdx+0x*]; movaps xmm2, xmm0; call 0x*")
        .Get();

    return bHasActorDensityValues;
}

bool FFortAthenaLivingWorldDensityGrid::IsInitialized() const
{
    static auto Flags = Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityGrid::bInitialized"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("test byte ptr [rcx+0x*], 0x*")
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, r12; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("test byte ptr [rcx+0x*], 0x*")
        .Disp();

    static auto Mask = (uint8)Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityGrid::bInitialized"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("test byte ptr [rcx+0x*], 0x*")
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, r12; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("test byte ptr [rcx+0x*], 0x*")
        .Imm();

    return Flags > 0 && (*((const uint8*)this + Flags) & Mask);
}

FIntVector FFortAthenaLivingWorldDensityGrid::GetCellCoordinates(const FVector& Location) const
{
    static auto GetCellCoordinates = Anchor::Function<FIntVector* (*)(const FFortAthenaLivingWorldDensityGrid*, FIntVector*, const TGameValue<FVector>*)>(TEXT("FFortSpatialGrid::GetCellCoordinates"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("movaps xmm2, xmm0; lea rdx, [rsp+0x*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("lea rcx, [rdi+0x*]; movaps xmm2, xmm0; lea rdx, [rsp+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    FIntVector Cell{ 0, 0, 0 };

    const TGameValue<FVector> GameLocation(Location);

    return *GetCellCoordinates(this, &Cell, &GameLocation);
}

bool FFortAthenaLivingWorldDensityGrid::AddActor(AActor* InActor, float DensityValue, float DensityRange)
{
    static auto NativeAddActor = Anchor::Optional<bool (*)(FFortAthenaLivingWorldDensityGrid*, AActor*, float, float)>(TEXT("FFortAthenaLivingWorldDensityGrid::AddActor"))
        .If(HasActorDensityValues())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:StartEncounter"))
        .Find("mov rdx, rax; mov r8, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("sqrtss xmm3, [*+0x*]; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(HasActorDensityValues())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:StartEncounter"))
        .Find("mov r9, *; mov r8, *; mov rdx, rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("sqrtss xmm3, [*+0x*]; xorps xmm2, xmm2; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(HasActorDensityValues())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:StartEncounter"))
        .Find("mov r9, *; mov r8, *; mov rdx, rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movss xmm3, dword ptr [0x*]; xorps xmm2, xmm2; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movaps xmm3, xmm6; movaps xmm2, xmm7; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(HasActorDensityValues())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:StartEncounter"))
        .Find("mov r9, *; mov r8, *; mov rdx, rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movss xmm3, dword ptr [0x*]; xorps xmm2, xmm2; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movaps xmm3, xmm6; movaps xmm2, xmm7; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (HasActorDensityValues())
    {
        if (!InActor || !NativeAddActor || !NativeAddActor(this, InActor, DensityValue, DensityRange))
            return false;

        UpdateActorDensityValue(InActor, DensityValue);

        if (GetActorDataDensityRange(InActor))
            return true;

        FActorDensityRange ActorDensityRange;
        ActorDensityRange.Actor = InActor;
        ActorDensityRange.DensityRange = DensityRange;

        ActorDensityRanges.Add(ActorDensityRange);

        return true;
    }

    if (!InActor || !IsInitialized())
        return false;

    auto& ActorToDataIndexMap = GetActorToDataIndexMap();
    const FObjectKey ActorKey(InActor);

    if (ActorToDataIndexMap.Find(ActorKey))
        return false;

    const FIntVector Cell = GetCellCoordinates(InActor->GetActorLocation());
    auto ActorDataAllocation = GetActorDataArray().AddUninitialized();
    auto ActorData = new (ActorDataAllocation) FFortAthenaLivingWorldDensityData();

    ActorData->Actor = InActor;
    ActorData->OccupiedCells.Add(Cell);

    ActorToDataIndexMap.Add(ActorKey, ActorDataAllocation.Index);
    GetCellToActorDataIndices().FindOrAdd(Cell).Add(ActorDataAllocation.Index);

    return true;
}

void FFortAthenaLivingWorldDensityGrid::RemoveActor(AActor* InActor)
{
    static auto NativeRemoveActor = Anchor::Function<void (*)(FFortAthenaLivingWorldDensityGrid*, AActor*)>(TEXT("FFortAthenaLivingWorldDensityGrid::RemoveActor"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rdx, rdi; mov rcx, r14; movaps xmm6, xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rdx, rdi; mov rcx, rbx; movaps xmm6, xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    NativeRemoveActor(this, InActor);
}

float FFortAthenaLivingWorldDensityGrid::GetActorDensityValue(AActor* InActor)
{
    static auto NativeGetActorDensityValue = Anchor::Function<float (*)(FFortAthenaLivingWorldDensityGrid*, AActor*)>(TEXT("FFortAthenaLivingWorldDensityGrid::GetActorDensityValue"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rcx, r14; mov rdi, rdx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rcx, rbx; mov rdi, rdx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return NativeGetActorDensityValue(this, InActor);
}

void FFortAthenaLivingWorldDensityGrid::UpdateActorDensityValue(AActor* InActor, float DensityValue)
{
    static auto NativeUpdateActorDensityValue = Anchor::Function<void (*)(FFortAthenaLivingWorldDensityGrid*, AActor*, float)>(TEXT("FFortAthenaLivingWorldDensityGrid::UpdateActorDensityValue"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rcx, r14; mov rdx, [rdx+0x*]; movaps xmm2, xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rsi+0x*]; mov rdx, [rdx+0x*]; movaps xmm2, xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [r14+0x*]; mov rdx, [rdx+0x*]; movaps xmm2, xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    NativeUpdateActorDensityValue(this, InActor, DensityValue);
}

float* FFortAthenaLivingWorldDensityGrid::GetActorDataDensityRange(AActor* InActor)
{
    static auto ActorDataSize = Anchor::Offset(TEXT("sizeof(FFortAthenaLivingWorldDensityData)"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rcx, rbx; mov rdi, rdx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("imul rcx, r10, 0x*")
        .Optional()
        .Imm();

    static auto DensityRange = Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityData::DensityRange"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:StartEncounter"))
        .Find("mov r9, *; mov r8, *; mov rdx, rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("sqrtss xmm3, [*+0x*]; xorps xmm2, xmm2; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rbp-0x*]; lea rbx, [rsi+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movss [rbx+0x*], xmm1; mov byte ptr [rbx+0x*], 0x0")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:StartEncounter"))
        .Find("mov r9, *; mov r8, *; mov rdx, rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movss xmm3, dword ptr [0x*]; xorps xmm2, xmm2; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movaps xmm3, xmm6; movaps xmm2, xmm7; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rbp-0x*]; lea rbx, [rsi+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movss [rbx+0x*], xmm1; mov byte ptr [rbx+0x*], 0x0")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:StartEncounter"))
        .Find("mov r9, *; mov r8, *; mov rdx, rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movss xmm3, dword ptr [0x*]; xorps xmm2, xmm2; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movaps xmm3, xmm6; movaps xmm2, xmm7; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rbp-0x*]; lea rbx, [rsi+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movss [rbx+0x*], xmm1; mov byte ptr [rbx+0x*], 0x0")
        .Optional()
        .Disp();

    if (!InActor || ActorDataSize <= 0 || DensityRange <= 0)
        return nullptr;

    auto ActorDataIndex = GetActorToDataIndexMap().Find(FObjectKey(InActor));

    return ActorDataIndex ? (float*)(*(uint8**)&GetActorDataArray() + *ActorDataIndex * ActorDataSize + DensityRange) : nullptr;
}

void FFortAthenaLivingWorldDensityGrid::UpdateActor(AActor* InActor)
{
    if (!InActor || !HasActorDensityValues())
        return;

    auto ActorDataDensityRange = GetActorDataDensityRange(InActor);
    float DensityRange = ActorDataDensityRange ? *ActorDataDensityRange : -1.0f;

    for (int32 Index = 0; !ActorDataDensityRange && Index < ActorDensityRanges.Num(); Index++)
    {
        if (ActorDensityRanges[Index].Actor.Get() != InActor)
            continue;

        DensityRange = ActorDensityRanges[Index].DensityRange;
        ActorDensityRanges.RemoveAtSwap(Index);
        break;
    }

    if (DensityRange < 0.0f)
        return;

    const float DensityValue = GetActorDensityValue(InActor);

    RemoveActor(InActor);
    AddActor(InActor, DensityValue, DensityRange);
}

void FFortAthenaLivingWorldDensityGrid::UpdateActors(int32 MaxActorUpdates)
{
    auto& ActorDataArray = GetActorDataArray();
    const int32 MaxIndex = ActorDataArray.GetMaxIndex();

    if (!IsInitialized() || !MaxIndex)
        return;

    for (int32 Count = 0; Count < MaxIndex && MaxActorUpdates > 0; Count++)
    {
        const int32 ActorDataIndex = NextActorUpdateIndex++ % MaxIndex;

        if (!ActorDataArray.IsAllocated(ActorDataIndex))
            continue;

        auto Actor = (AActor*)ActorDataArray[ActorDataIndex].Actor.Get();

        if (!Actor)
            continue;

        SetActorDataCell(ActorDataIndex, GetCellCoordinates(Actor->GetActorLocation()));
        MaxActorUpdates--;
    }
}

void FFortAthenaLivingWorldDensityGrid::SetActorDataCell(int32 ActorDataIndex, const FIntVector& Cell)
{
    auto& OccupiedCells = GetActorDataArray()[ActorDataIndex].OccupiedCells;

    if (OccupiedCells.Num() == 1 && OccupiedCells[0] == Cell)
        return;

    auto& CellToActorDataIndices = GetCellToActorDataIndices();

    for (auto& OccupiedCell : OccupiedCells)
    {
        auto ActorDataIndices = CellToActorDataIndices.Find(OccupiedCell);

        if (!ActorDataIndices)
            continue;

        ActorDataIndices->RemoveSingleSwap(ActorDataIndex);

        if (!ActorDataIndices->Num())
            CellToActorDataIndices.Remove(OccupiedCell);
    }

    OccupiedCells.Empty();
    OccupiedCells.Add(Cell);
    CellToActorDataIndices.FindOrAdd(Cell).Add(ActorDataIndex);
}

TMap<FIntVector, TArray<int32>>& FFortAthenaLivingWorldDensityGrid::GetCellToActorDataIndices()
{
    static auto CellToActorDataIndices = Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityGrid::CellToActorDataIndices"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rbx+0x*]; mov r8, rdi; lea rdx, [rsp+0x*]")
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, r12; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rbx+0x*]; mov r8, rdi; lea rdx, [rsp+0x*]")
        .Disp();

    return *(TMap<FIntVector, TArray<int32>>*)((uint8*)this + CellToActorDataIndices);
}

TMap<FObjectKey, int32>& FFortAthenaLivingWorldDensityGrid::GetActorToDataIndexMap()
{
    static auto ActorToDataIndexMap = Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityGrid::ActorToDataIndexMap"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rbx+0x*]; mov rcx, *; mov [rsp+0x*], rax")
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, r12; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rbx+0x*]; mov rcx, *; mov [rsp+0x*], rax")
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rcx, rbx; mov rdi, rdx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rdi, [rcx+0x*]; lea rcx, [rsp+0x*]; call 0x*")
        .Disp();

    return *(TMap<FObjectKey, int32>*)((uint8*)this + ActorToDataIndexMap);
}

TSparseArray<FFortAthenaLivingWorldDensityData>& FFortAthenaLivingWorldDensityGrid::GetActorDataArray()
{
    static auto ActorDataArray = Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityGrid::ActorDataArray"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rbx+0x*]; mov rax, [*]; mov rdi, [rax+rcx*8+0x8]")
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, r12; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rbx+0x*]; mov rax, [*]; mov rdi, [rax+rcx*8+0x8]")
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rcx, rbx; mov rdi, rdx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rbx+0x*]; mov edx, r10d; call 0x*")
        .Disp();

    return *(TSparseArray<FFortAthenaLivingWorldDensityData>*)((uint8*)this + ActorDataArray);
}
