#include "pch.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "CoreGlobals.h"
#include "Logging/LogMacros.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Config.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UWorld::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.World"));

    return Class;
}

AGameModeBase* UWorld::GetAuthGameMode() const
{
    static auto AuthorityGameMode = GetClass()->FindPropertyByName(FName(TEXT("AuthorityGameMode")));

    return *AuthorityGameMode->ContainerPtrToValuePtr<AGameModeBase*>(this);
}

AGameStateBase* UWorld::GetGameState() const
{
    static auto GameState = GetClass()->FindPropertyByName(FName(TEXT("GameState")));

    return *GameState->ContainerPtrToValuePtr<AGameStateBase*>(this);
}

UObject* UWorld::GetSubsystemBase(UClass* SubsystemClass) const
{
    static auto SubsystemCollection = Anchor::Offset(TEXT("UWorld::SubsystemCollection"))
        .Exec(TEXT("/Script/Engine.SubsystemBlueprintLibrary:GetWorldSubsystem"))
        .Find("call [rax+0x*]; test rax, rax; jz 0x*; lea *, [rax+0x*]")
        .Find("lea *, [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.SubsystemBlueprintLibrary:GetWorldSubsystem"))
        .Find("call [rax+0x*]; test rax, rax; jz 0x*; mov rdx, r*; mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea r*, [rcx+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.SubsystemBlueprintLibrary:GetWorldSubsystem"))
        .Find("call [rax+0x*]; mov *, rax; test rax, rax; jz 0x*")
        .Find("lea rdx, [rbp-0x*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea r*, [rcx+0x*]")
        .Disp();
    static auto GetSubsystemInternal = Anchor::Function<UObject* (*)(void*, UClass*)>(TEXT("FSubsystemCollectionBase::GetSubsystemInternal"))
        .Exec(TEXT("/Script/Engine.SubsystemBlueprintLibrary:GetWorldSubsystem"))
        .Find("call [rax+0x*]; test rax, rax; jz 0x*; lea *, [rax+0x*]")
        .Find("lea *, [rax+0x*]")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.SubsystemBlueprintLibrary:GetWorldSubsystem"))
        .Find("call [rax+0x*]; test rax, rax; jz 0x*; mov rdx, r*; mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("jmp 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.SubsystemBlueprintLibrary:GetWorldSubsystem"))
        .Find("call [rax+0x*]; mov *, rax; test rax, rax; jz 0x*")
        .Find("lea rdx, [rbp-0x*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("jmp 0x*")
        .Target()
        .Get();

    return GetSubsystemInternal((uint8*)this + SubsystemCollection, SubsystemClass);
}

UAISystemBase* UWorld::GetAISystem() const
{
    static auto AISystem = GetClass()->FindPropertyByName(FName(TEXT("AISystem")));

    return *AISystem->ContainerPtrToValuePtr<UAISystemBase*>(this);
}

UGameInstance* UWorld::GetGameInstance() const
{
    static auto OwningGameInstance = GetClass()->FindPropertyByName(FName(TEXT("OwningGameInstance")));

    return *OwningGameInstance->ContainerPtrToValuePtr<UGameInstance*>(this);
}

AGameNetworkManager* UWorld::GetNetworkManager() const
{
    static auto NetworkManager = GetClass()->FindPropertyByName(FName(TEXT("NetworkManager")));

    return *NetworkManager->ContainerPtrToValuePtr<AGameNetworkManager*>(this);
}

ULevel* UWorld::GetPersistentLevel() const
{
    static auto PersistentLevel = GetClass()->FindPropertyByName(FName(TEXT("PersistentLevel")));

    return *PersistentLevel->ContainerPtrToValuePtr<ULevel*>(this);
}

ULevel* UWorld::GetCurrentLevelPendingVisibility() const
{
    static auto CurrentLevelPendingVisibility = GetClass()->FindPropertyByName(FName(TEXT("CurrentLevelPendingVisibility")));

    return *CurrentLevelPendingVisibility->ContainerPtrToValuePtr<ULevel*>(this);
}

const TArray<ULevelStreaming*>& UWorld::GetStreamingLevels() const
{
    static auto StreamingLevels = GetClass()->FindPropertyByName(FName(TEXT("StreamingLevels")));

    return *StreamingLevels->ContainerPtrToValuePtr<TArray<ULevelStreaming*>>(this);
}

ULevel* UWorld::GetCurrentLevelPendingInvisibility() const
{
    static auto CurrentLevelPendingInvisibility = GetClass()->FindPropertyByName(FName(TEXT("CurrentLevelPendingInvisibility")));

    return *CurrentLevelPendingInvisibility->ContainerPtrToValuePtr<ULevel*>(this);
}

float UWorld::GetTimeSeconds() const
{
    static auto TimeSeconds = Anchor::Offset(TEXT("UWorld::TimeSeconds"))
        .Native(TEXT("/Script/Engine.GameplayStatics:GetTimeSeconds"))
        .Find("movss xmm0, dword ptr [rax+*]")
        .Or()
        .Native(TEXT("/Script/Engine.GameplayStatics:GetTimeSeconds"))
        .Find("movsd xmm0, qword ptr [rax+*]")
        .Or()
        .Exec(TEXT("/Script/Engine.GameplayStatics:GetTimeSeconds"))
        .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
        .Find("movsd xmm0, qword ptr [rax+*]")
        .Disp();
    static auto bDoubleTimeSeconds = Anchor::Exists()
        .Native(TEXT("/Script/Engine.GameplayStatics:GetTimeSeconds"))
        .Find("movsd xmm0, qword ptr [rax+*]")
        .Or()
        .Exec(TEXT("/Script/Engine.GameplayStatics:GetTimeSeconds"))
        .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
        .Find("movsd xmm0, qword ptr [rax+*]")
        .Get();

    return bDoubleTimeSeconds ? (float)*(double*)((uint8_t*)this + TimeSeconds) : *(float*)((uint8_t*)this + TimeSeconds);
}

AWorldSettings* UWorld::GetWorldSettings(bool bCheckStreamingPesistent, bool bChecked) const
{
    static auto GetWorldSettings = Anchor::Function<AWorldSettings* (*)(const UWorld*, bool, bool)>(TEXT("UWorld::GetWorldSettings"))
        .Native(TEXT("/Script/Engine.GameplayStatics:GetGlobalTimeDilation"))
        .Call(2)
        .Get();

    return GetWorldSettings(this, bCheckStreamingPesistent, bChecked);
}

UNetDriver* UWorld::GetNetDriver() const
{
    static auto NetDriver = GetClass()->FindPropertyByName(FName(TEXT("NetDriver")));

    return *NetDriver->ContainerPtrToValuePtr<UNetDriver*>(this);
}

UNetDriver* UWorld::GetDemoNetDriver() const
{
    static auto DemoNetDriver = GetClass()->FindPropertyByName(FName(TEXT("DemoNetDriver")));

    return *DemoNetDriver->ContainerPtrToValuePtr<UNetDriver*>(this);
}

ENetMode UWorld::InternalGetNetMode() const
{
    DETOUR(UWorld, InternalGetNetMode, Anchor::Hook()
        .String(TEXT("Failed to determine if OSS is server in PIE, OSS requests will fail"))
        .Start()
        .Find("mov rcx, rax; call 0x*")
        .Call()
        .Or()
        .String(TEXT("Failed to determine if OSS is server in PIE, OSS requests will fail"))
        .Start()
        .Find("mov rcx, *; call 0x*; mov ebx, eax; lea eax, [rbx-0x1]")
        .Call()
        .Or()
        .Pattern("48 83 EC 28 48 83 79 38 00 4C 8B C1 75 ? 48 8B 91 F0 00 00 00")
        .Or()
        .Pattern("48 83 EC 28 48 83 79 38 00 48 8B D1 75 ? 48 8B 89 ? ? 00 00 48 85 C9 74 ? 80 B9 ? ? 00 00 00 75 ? 48 8B 81 ? ? 00 00 48 85 C0 74 ? 83 B8 34 01 00 00 01 74 ? 48 8B 01 FF 90 ? ? 00 00 84 C0 0F 85 ? ? ? ? B8 03 00 00 00 48 83 C4 28 C3")
        .Or()
        .Pattern("56 48 83 EC 20 BE 03 00 00 00 48 83 79 ? 00 75 ? 48 89 C8 48 8B 89 ? ? 00 00 48 85 C9 74 ? 80 B9 ? ? 00 00 00 75")
        .Or()
        .Pattern("48 83 EC 28 48 83 79 38 00 48 8B D1 74 ? B8 03 00 00 00 48 83 C4 28 C3 48 8B 89 ? ? 00 00")
        .Inline());

    if (GetNetDriver() != nullptr)
    {
        return GetNetDriver()->GetNetMode();
    }

    return Original();
}

ENetMode UWorld::AttemptDeriveFromURL() const
{
    DETOUR(UWorld, AttemptDeriveFromURL, Anchor::Hook()
        .String(TEXT("Failed to determine if OSS is server in PIE, OSS requests will fail"))
        .Start()
        .Find("mov rcx, rax; call 0x*")
        .Call()
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Or()
        .String(TEXT("Failed to determine if OSS is server in PIE, OSS requests will fail"))
        .Start()
        .Find("mov rcx, *; call 0x*; mov ebx, eax; lea eax, [rbx-0x1]")
        .Call()
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Or()
        .Pattern("48 83 EC 28 48 83 79 38 00 4C 8B C1 75 ? 48 8B 91 F0 00 00 00")
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Or()
        .Pattern("48 83 EC 28 48 83 79 38 00 48 8B D1 75 ? 48 8B 89 ? ? 00 00 48 85 C9 74 ? 80 B9 ? ? 00 00 00 75 ? 48 8B 81 ? ? 00 00 48 85 C0 74 ? 83 B8 34 01 00 00 01 74 ? 48 8B 01 FF 90 ? ? 00 00 84 C0 0F 85 ? ? ? ? B8 03 00 00 00 48 83 C4 28 C3")
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Or()
        .Pattern("56 48 83 EC 20 BE 03 00 00 00 48 83 79 ? 00 75 ? 48 89 C8 48 8B 89 ? ? 00 00 48 85 C9 74 ? 80 B9 ? ? 00 00 00 75")
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Or()
        .Pattern("48 83 EC 28 48 83 79 38 00 48 8B D1 74 ? B8 03 00 00 00 48 83 C4 28 C3 48 8B 89 ? ? 00 00")
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Optional()
        .Inline());

    if (GetNetDriver() != nullptr)
    {
        return GetNetDriver()->GetNetMode();
    }

    return Original();
}

PATCH(UWorld, PatchInlinedNetMode, UWorld::PatchInlinedNetMode())

int32 UWorld::PatchInlinedNetMode()
{
    static auto TailCall = Anchor::Data<const uint8*>(TEXT("UWorld::InternalGetNetMode tail call"))
        .String(TEXT("Failed to determine if OSS is server in PIE, OSS requests will fail"))
        .Start()
        .Find("mov rcx, rax; call 0x*")
        .Call()
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Or()
        .String(TEXT("Failed to determine if OSS is server in PIE, OSS requests will fail"))
        .Start()
        .Find("mov rcx, *; call 0x*; mov ebx, eax; lea eax, [rbx-0x1]")
        .Call()
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Or()
        .Pattern("48 83 EC 28 48 83 79 38 00 4C 8B C1 75 ? 48 8B 91 F0 00 00 00")
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Or()
        .Pattern("48 83 EC 28 48 83 79 38 00 48 8B D1 75 ? 48 8B 89 ? ? 00 00 48 85 C9 74 ? 80 B9 ? ? 00 00 00 75 ? 48 8B 81 ? ? 00 00 48 85 C0 74 ? 83 B8 34 01 00 00 01 74 ? 48 8B 01 FF 90 ? ? 00 00 84 C0 0F 85 ? ? ? ? B8 03 00 00 00 48 83 C4 28 C3")
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Or()
        .Pattern("56 48 83 EC 20 BE 03 00 00 00 48 83 79 ? 00 75 ? 48 89 C8 48 8B 89 ? ? 00 00 48 85 C9 74 ? 80 B9 ? ? 00 00 00 75")
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Or()
        .Pattern("48 83 EC 28 48 83 79 38 00 48 8B D1 74 ? B8 03 00 00 00 48 83 C4 28 C3 48 8B 89 ? ? 00 00")
        .Find("add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Optional()
        .Get();

    static auto NetDriver = StaticClass()->FindPropertyByName(FName(TEXT("NetDriver")));

    if (!TailCall || !NetDriver || NetDriver->GetOffset_ForInternal() >= 0x80)
        return 0;

    auto AttemptDeriveFromURL = Anchor::FCode(TailCall).Target();

    char CompareText[48];
    char RegisterCompareText[48];
    char CallText[40];
    char JumpText[40];
    snprintf(CompareText, sizeof(CompareText), "cmp qword ptr [*+0x%x], *", (uint32)NetDriver->GetOffset_ForInternal());
    snprintf(RegisterCompareText, sizeof(RegisterCompareText), "cmp [*+0x%x], *", (uint32)NetDriver->GetOffset_ForInternal());
    snprintf(CallText, sizeof(CallText), "call 0x%llx", (unsigned long long)AttemptDeriveFromURL);
    snprintf(JumpText, sizeof(JumpText), "jmp 0x%llx", (unsigned long long)AttemptDeriveFromURL);

    auto Image = Anchor::Base();
    auto Headers = (IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew);
    auto Sections = IMAGE_FIRST_SECTION(Headers);
    int32 NumPatched = 0;

    for (int32 Index = 0; Index < Headers->FileHeader.NumberOfSections; Index++)
    {
        if (!(Sections[Index].Characteristics & IMAGE_SCN_MEM_EXECUTE))
            continue;

        for (auto Address = Image + Sections[Index].VirtualAddress; Address + 0x10 < Image + Sections[Index].VirtualAddress + Sections[Index].Misc.VirtualSize; Address++)
        {
            if ((Address[0] & 0xF8) != 0x48 || (Address[1] != 0x83 && Address[1] != 0x39) || (Address[2] & 0xC0) != 0x40 || Address[(Address[2] & 0x7) == 0x4 ? 4 : 3] != (uint8)NetDriver->GetOffset_ForInternal())
                continue;

            Anchor::FCode Compare(Address);

            if (!Compare.Is(CompareText) && !Compare.Is(RegisterCompareText))
                continue;

            if (Address < TailCall && TailCall - Address < 0x100)
                continue;

            auto Branch = Compare.Find("*");

            for (int32 Count = 0; Count < 6 && Branch.Get() && !Branch.Is("jz 0x*") && !Branch.Is("jnz 0x*") && !Branch.Is("jmp *") && !Branch.Is("call *") && !Branch.Is("ret*"); Count++)
                Branch = Branch.Find("*");

            if (!Branch.Get() || (!Branch.Is("jz 0x*") && !Branch.Is("jnz 0x*")))
                continue;

            Anchor::FCode Code(Branch.Is("jz 0x*") ? Branch.Target() : Branch.Find("*").Get());
            bool bReachesFallback = false;

            for (int32 Count = 0; Count < 14 && Code.Get() && !bReachesFallback; Count++)
            {
                if (Code.Is(CallText) || Code.Is(JumpText))
                {
                    bReachesFallback = true;
                }
                else if (Code.Is("jmp *") || Code.Is("ret*") || Code.Is("int3"))
                {
                    break;
                }
                else if (Code.Is("jz 0x*") || Code.Is("jnz 0x*"))
                {
                    Anchor::FCode Target(Code.Target());

                    for (int32 TargetCount = 0; TargetCount < 6 && Target.Get() && !bReachesFallback; TargetCount++)
                    {
                        bReachesFallback = Target.Is(CallText) || Target.Is(JumpText);

                        if (Target.Is("jmp *") || Target.Is("ret*") || Target.Is("int3"))
                            break;

                        Target = Target.Find("*");
                    }
                }

                Code = Code.Find("*");
            }

            if (!bReachesFallback)
                continue;

            if (Branch.Is("jz 0x*"))
                *Branch.Get() == 0x0F ? Anchor::Patch(TEXT("UWorld::InternalGetNetMode inlined"), Branch.Get(), { 0x90, 0xE9 }) : Anchor::Patch(TEXT("UWorld::InternalGetNetMode inlined"), Branch.Get(), { 0xEB });
            else
                *Branch.Get() == 0x0F ? Anchor::Patch(TEXT("UWorld::InternalGetNetMode inlined"), Branch.Get(), { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }) : Anchor::Patch(TEXT("UWorld::InternalGetNetMode inlined"), Branch.Get(), { 0x90, 0x90 });

            NumPatched++;
        }
    }

    UE_LOG(LogVelora, Log, TEXT("Patched %d inlined UWorld::InternalGetNetMode"), NumPatched);

    return NumPatched;
}

AServerStreamingLevelsVisibility* UWorld::GetServerStreamingLevelsVisibility() const
{
    static auto ServerStreamingLevelsVisibility = GetClass()->FindPropertyByName(FName(TEXT("ServerStreamingLevelsVisibility")));

    return ServerStreamingLevelsVisibility ? *ServerStreamingLevelsVisibility->ContainerPtrToValuePtr<AServerStreamingLevelsVisibility*>(this) : nullptr;
}

void UWorld::SetServerStreamingLevelsVisibility(AServerStreamingLevelsVisibility* InServerStreamingLevelsVisibility)
{
    static auto ServerStreamingLevelsVisibility = GetClass()->FindPropertyByName(FName(TEXT("ServerStreamingLevelsVisibility")));

    if (ServerStreamingLevelsVisibility)
        *ServerStreamingLevelsVisibility->ContainerPtrToValuePtr<AServerStreamingLevelsVisibility*>(this) = InServerStreamingLevelsVisibility;
}

AServerStreamingLevelsVisibility* AServerStreamingLevelsVisibility::SpawnServerActor(const UWorld* World)
{
    static auto SpawnServerActor = Anchor::Function<AServerStreamingLevelsVisibility* (*)(const UWorld*)>(TEXT("AServerStreamingLevelsVisibility::SpawnServerActor"))
        .String(TEXT("sorting actor list"))
        .Find("test *, 0x60000000; jnz *; mov r8b, 0x1; lea rdx, [*]; call 0x*")
        .Call()
        .Reference("call 0x*; add *, 0x8")
        .Start()
        .Or()
        .Pattern("48 83 EC 28 80 B9 ? ? 00 00 03 75 ? 48 81 C1 ? ? 00 00 45 84 C0 74 ? 4C 8B C2 45 33 C9")
        .Reference("call 0x*; add *, 0x8")
        .Start()
        .Or()
        .String(TEXT("sorting actor list"))
        .Start()
        .Find("mov r8b, 0x1; mov rcx, [*+0xc0]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Reference("call 0x*; add *, 0x8")
        .Start()
        .Or()
        .String(TEXT("ServerStreamingLevelsVisibility"))
        .Start()
        .Reference("call 0x*; mov ecx, [rax+0x*]")
        .Start()
        .Optional()
        .Get();

    return SpawnServerActor ? SpawnServerActor(World) : nullptr;
}

void UWorld::SetNetDriver(UNetDriver* NewDriver)
{
    static auto NetDriver = GetClass()->FindPropertyByName(FName(TEXT("NetDriver")));

    *NetDriver->ContainerPtrToValuePtr<UNetDriver*>(this) = NewDriver;
}

void UWorld::SetNextSwitchCountdown(float InNextSwitchCountdown)
{
    static auto NextSwitchCountdown = Anchor::Offset(TEXT("UWorld::NextSwitchCountdown"))
        .String(TEXT("Server switch level: %s"))
        .Start()
        .Find("subss *")
        .Find("movss [*], xmm*")
        .Or()
        .String(TEXT("Server switch level: %s"))
        .Last("subss *")
        .Find("movss [*], xmm*")
        .Disp();

    if (NextSwitchCountdown > 0)
        *(float*)((uint8_t*)this + NextSwitchCountdown) = InNextSwitchCountdown;
}

FLevelCollection* UWorld::FindCollectionByType(const ELevelCollectionType InType)
{
    static auto FindCollectionByType = Anchor::Optional<FLevelCollection* (*)(UWorld*, ELevelCollectionType)>(TEXT("UWorld::FindCollectionByType"))
        .String(TEXT("RecordReplay: failed to create demo net driver!"))
        .Find("xor edx, edx; mov rcx, *")
        .Call()
        .Get();

    if (FindCollectionByType)
        return FindCollectionByType(this, InType);

    static auto LevelCollections = Anchor::Offset(TEXT("UWorld::LevelCollections"))
        .String(TEXT("RecordReplay: failed to create demo net driver!"))
        .Find("call r8")
        .Find("call [rax+0x*]")
        .Find("movsxd rax, dword ptr [r*+0x*]")
        .Find("mov rcx, [r*+0x*]")
        .Or()
        .String(TEXT("RecordReplay: failed to create demo net driver!"))
        .Start()
        .Find("movsxd *, dword ptr [*+0x*]; test *, *; jz *; mov *, [*+0x*]; imul *, *, 0x*; cmp byte ptr [*], 0x0")
        .Find("mov *, [*+0x*]")
        .Disp();

    static auto LevelCollectionSize = Anchor::Offset(TEXT("FLevelCollection size"))
        .String(TEXT("RecordReplay: failed to create demo net driver!"))
        .Find("call r8")
        .Find("call [rax+0x*]")
        .Find("imul r*, r*, 0x*")
        .Or()
        .String(TEXT("RecordReplay: failed to create demo net driver!"))
        .Start()
        .Find("movsxd *, dword ptr [*+0x*]; test *, *; jz *; mov *, [*+0x*]; imul *, *, 0x*; cmp byte ptr [*], 0x0")
        .Find("imul *, *, 0x*")
        .Imm();

    if (LevelCollections <= 0 || LevelCollectionSize <= 0)
        return nullptr;

    auto& Collections = *(TArray<uint8>*)((uint8*)this + LevelCollections);

    for (int32 Index = 0; Index < Collections.Num(); Index++)
    {
        auto Collection = (FLevelCollection*)(Collections.GetData() + Index * LevelCollectionSize);

        if (Collection->GetType() == InType)
            return Collection;
    }

    return nullptr;
}

void UWorld::InitializeActorsForPlay(const FURL& InURL, bool bResetTime, FRegisterComponentContext* Context)
{
    DETOUR(UWorld, InitializeActorsForPlay, Anchor::Hook()
        .String(TEXT("Bringing up level for play took: %f"))
        .Start()
        .Optional()
        .Inline());

    if (!GIsClient && !GetNetDriver())
        Listen(const_cast<FURL&>(InURL));

    Original(InURL, bResetTime, Context);
}

bool UWorld::Listen(FURL& InURL)
{
    DETOUR(UWorld, Listen, Anchor::Hook()
        .String(TEXT("LoadMap: failed to Listen(%s)"))
        .Last("call 0x*; test al, al")
        .Is("call 0x*; test al, al; jnz *")
        .Optional()
        .AtCall());

    if (GetNetDriver())
    {
        GEngine->BroadcastNetworkFailure(this, GetNetDriver(), ENetworkFailure::NetDriverAlreadyExists);
        return false;
    }

    if (GEngine->CreateNamedNetDriver(this, NAME_GameNetDriver, NAME_GameNetDriver))
    {
        SetNetDriver(GEngine->FindNamedNetDriver(this, NAME_GameNetDriver));
        GetNetDriver()->SetWorld(this);

        FLevelCollection* const SourceCollection = FindCollectionByType(ELevelCollectionType::DynamicSourceLevels);

        if (SourceCollection)
            SourceCollection->SetNetDriver(GetNetDriver());

        FLevelCollection* const StaticCollection = FindCollectionByType(ELevelCollectionType::StaticLevels);

        if (StaticCollection)
            StaticCollection->SetNetDriver(GetNetDriver());
    }

    if (GetNetDriver() == nullptr)
    {
        GEngine->BroadcastNetworkFailure(this, nullptr, ENetworkFailure::NetDriverCreateFailure);
        return false;
    }

    FString Error;

    InURL.Port = Config::Port;

    if (!GetNetDriver()->InitListen(this, InURL, false, Error))
    {
        GEngine->BroadcastNetworkFailure(this, GetNetDriver(), ENetworkFailure::NetDriverListenFailure, Error);
        static auto& LogWorld = *Anchor::Data<FLogCategoryBase*>(TEXT("LogWorld"))
            .String(TEXT("Bringing %s up for play (max tick rate %i) at %s"))
            .Last("movzx *, byte ptr [0x*]")
            .Target()
            .Get();

        UE_LOG(LogWorld, Log, TEXT("Failed to listen: %s"), *Error);
        GetNetDriver()->SetWorld(nullptr);
        SetNetDriver(nullptr);

        FLevelCollection* SourceCollection = FindCollectionByType(ELevelCollectionType::DynamicSourceLevels);

        if (SourceCollection)
            SourceCollection->SetNetDriver(nullptr);

        FLevelCollection* StaticCollection = FindCollectionByType(ELevelCollectionType::StaticLevels);

        if (StaticCollection)
            StaticCollection->SetNetDriver(nullptr);

        return false;
    }

    static bool LanPlay = FParse::Param(FCommandLine::Get(), TEXT("lanplay"));

    if (!LanPlay && (GetNetDriver()->GetMaxInternetClientRate() < GetNetDriver()->GetMaxClientRate()) && (GetNetDriver()->GetMaxInternetClientRate() > 2500))
        GetNetDriver()->SetMaxClientRate(GetNetDriver()->GetMaxInternetClientRate());

    GetNetDriver()->SetClampListenServerTickRate(true);

    SetNextSwitchCountdown(GetNetDriver()->GetServerTravelPause());

    return true;
}

bool UWorld::AreActorsInitialized() const
{
    static auto AreActorsInitialized = Anchor::Function<bool (*)(const UWorld*)>(TEXT("UWorld::AreActorsInitialized"))
        .Pattern("33 C0 38 81 ? ? 00 00 7D ? 48 8B 49 30 48 85 C9 74 ? 39 81 ? ? 00 00 74 ? B0 01 C3")
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortTimeOfDayManager:UpdatePostProcessVolumes"))
        .Find("mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return AreActorsInitialized(this);
}

UWorld::FOnActorsInitialized& UWorld::GetOnActorsInitialized()
{
    static auto OnActorsInitialized = Anchor::Offset(TEXT("UWorld::OnActorsInitialized"))
        .String(TEXT("Bringing up level for play took: %f"))
        .Last("lea rcx, [*+0x*]; lea rdx, [*]; mov [*], *; mov [*], *; call 0x*; lea rdx, [*]; lea rcx, [0x*]; call 0x*")
        .Or()
        .String(TEXT("Bringing up level for play took: %f"))
        .Last("lea rcx, [*+0x*]; mov [*], *; lea rdx, [*]; mov [*], *; call 0x*; lea rdx, [*]; lea rcx, [0x*]; call 0x*")
        .Disp();

    return *(FOnActorsInitialized*)((uint8*)this + OnActorsInitialized);
}
