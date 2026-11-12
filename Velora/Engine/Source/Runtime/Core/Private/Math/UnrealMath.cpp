#include "pch.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"
#include "Math/TransformVectorized.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

FVector FRotator::Vector() const
{
    static auto Vector = Anchor::Function<TGameValue<FVector>* (*)(const TGameValue<FRotator>&, TGameValue<FVector>*)>(TEXT("FRotator::Vector"))
        .Exec(TEXT("/Script/Engine.KismetMathLibrary:Conv_RotatorToVector"))
        .Find("setnz *; add r*, r*; mov [*+0x20], *; call 0x*; movaps xmm2, xmm0")
        .Pattern("48 89 5C 24 08 57 48 83 EC 40 F2 0F 10 0D ? ? ? ? 48 8B FA F2 0F 10 01 48 8B D9 44 0F 29 44 24 30 44 0F 29 4C 24 20 E8 ? ? ? ? 0F 28 D0 0F 57 C0 F2 0F 59 15 ? ? ? ? F2 0F 10 C2 E8")
        .Or()
        .Native(TEXT("/Script/Engine.KismetMathLibrary:Conv_RotatorToVector"))
        .Get();

    TGameValue<FVector> Result;

    return Vector(*this, &Result)->Get();
}

FRotator FVector::ToOrientationRotator() const
{
    static auto ToOrientationRotator = Anchor::Function<TGameValue<FRotator>* (*)(const TGameValue<FVector>&, TGameValue<FRotator>*)>(TEXT("FVector::ToOrientationRotator"))
        .Exec(TEXT("/Script/Engine.KismetMathLibrary:Conv_VectorToRotator"))
        .Find("cmp *, 0x1; sbb *, 0xffffffffffffffff; mov [*+0x20], *")
        .Native(TEXT("/Script/Engine.KismetMathLibrary:Conv_Vector4ToRotator"))
        .Or()
        .Native(TEXT("/Script/Engine.KismetMathLibrary:Conv_VectorToRotator"))
        .Get();

    TGameValue<FRotator> Result;

    return ToOrientationRotator(*this, &Result)->Get();
}

FQuat FRotator::Quaternion() const
{
    float SP = sinf(Pitch * 0.008726646f);
    float CP = cosf(Pitch * 0.008726646f);
    float SY = sinf(Yaw * 0.008726646f);
    float CY = cosf(Yaw * 0.008726646f);
    float SR = sinf(Roll * 0.008726646f);
    float CR = cosf(Roll * 0.008726646f);

    FQuat RotationQuat;
    RotationQuat.X = CR * SP * SY - SR * CP * CY;
    RotationQuat.Y = -CR * SP * CY - SR * CP * SY;
    RotationQuat.Z = CR * CP * SY - SR * SP * CY;
    RotationQuat.W = CR * CP * CY + SR * SP * SY;

    return RotationQuat;
}

FRotator FQuat::Rotator() const
{
    static auto Rotator = Anchor::Function<TGameValue<FRotator>* (*)(const TGameValue<FQuat>&, TGameValue<FRotator>*)>(TEXT("FQuat::Rotator"))
        .If(::Anchor::Exists().Exec(TEXT("/Script/Engine.SceneComponent:K2_GetComponentRotation")).Find("setnz *").Find("add [*+0x20], *").Find("call 0x*").Target().Has("movups xmm*, [rcx+0x1c0]").Or().Exec(TEXT("/Script/Engine.SceneComponent:K2_GetComponentRotation")).Find("setnz *").Find("add [*+0x20], *").Find("call 0x*").Target().Has("movups xmm*, [rcx+0x1d0]").Get())
        .Native(TEXT("/Script/Engine.KismetMathLibrary:Quat_Rotator"))
        .Or()
        .If(!::Anchor::Exists().Exec(TEXT("/Script/Engine.SceneComponent:K2_GetComponentRotation")).Find("cmp *, 0x1; sbb *, 0xffffffffffffffff; mov [*+0x20], *").Get())
        .Exec(TEXT("/Script/Engine.SceneComponent:K2_GetComponentRotation"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/Engine.SceneComponent:K2_GetComponentRotation"))
        .Call()
        .Or()
        .Native(TEXT("/Script/Engine.KismetMathLibrary:Quat_Rotator"))
        .Get();

    TGameValue<FRotator> Result;

    return Rotator(*this, &Result)->Get();
}

const FTransform FTransform::Identity;

const FQuat FQuat::Identity(0, 0, 0, 1);

FVector FMath::VRand()
{
    static auto VRand = Anchor::Function<TGameValue<FVector>* (*)(TGameValue<FVector>*)>(TEXT("UKismetMathLibrary::RandomUnitVector"))
        .Native(TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVector"))
        .Get();

    TGameValue<FVector> Result;

    return VRand(&Result)->Get();
}

FVector FMath::VRandCone(FVector const& Dir, float ConeHalfAngleRad)
{
    static auto VRandCone = Anchor::Function<TGameValue<FVector>* (*)(TGameValue<FVector>*, const TGameValue<FVector>&, float)>(TEXT("FMath::VRandCone"))
        .Native(TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVectorInCone"))
        .Or()
        .Native(TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVectorInConeInRadians"))
        .Get();

    TGameValue<FVector> Result;

    return VRandCone(&Result, Dir, ConeHalfAngleRad)->Get();
}

bool UE::LWC::IsEnabled()
{
    static auto bEnabled = []
    {
        auto Vector = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CoreUObject.Vector"));

        return Vector && Vector->GetPropertiesSize() == (int32)sizeof(FVector3d);
    }();

    return bEnabled;
}
