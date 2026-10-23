#pragma once

#include "HAL/Platform.h"
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/UnrealMathUtility.h"
#include <xmmintrin.h>

typedef __m128 VectorRegister;

FORCEINLINE VectorRegister VectorCross(const VectorRegister& Vec1, const VectorRegister& Vec2)
{
    VectorRegister A_YZXW = _mm_shuffle_ps(Vec1, Vec1, _MM_SHUFFLE(3, 0, 2, 1));
    VectorRegister B_ZXYW = _mm_shuffle_ps(Vec2, Vec2, _MM_SHUFFLE(3, 1, 0, 2));
    VectorRegister A_ZXYW = _mm_shuffle_ps(Vec1, Vec1, _MM_SHUFFLE(3, 1, 0, 2));
    VectorRegister B_YZXW = _mm_shuffle_ps(Vec2, Vec2, _MM_SHUFFLE(3, 0, 2, 1));

    return _mm_sub_ps(_mm_mul_ps(A_YZXW, B_ZXYW), _mm_mul_ps(A_ZXYW, B_YZXW));
}

FORCEINLINE VectorRegister VectorQuaternionRotateVector(const VectorRegister& Quat, const VectorRegister& VectorW0)
{
    const VectorRegister QW = _mm_shuffle_ps(Quat, Quat, _MM_SHUFFLE(3, 3, 3, 3));
    VectorRegister T = VectorCross(Quat, VectorW0);
    T = _mm_add_ps(T, T);

    const VectorRegister VTemp0 = _mm_add_ps(_mm_mul_ps(QW, T), VectorW0);
    const VectorRegister VTemp1 = VectorCross(Quat, T);
    const VectorRegister Rotated = _mm_add_ps(VTemp0, VTemp1);

    return Rotated;
}

FORCEINLINE VectorRegister VectorQuaternionMultiply2(const VectorRegister& Quat1, const VectorRegister& Quat2)
{
    const float X1 = Quat1.m128_f32[0], Y1 = Quat1.m128_f32[1], Z1 = Quat1.m128_f32[2], W1 = Quat1.m128_f32[3];
    const float X2 = Quat2.m128_f32[0], Y2 = Quat2.m128_f32[1], Z2 = Quat2.m128_f32[2], W2 = Quat2.m128_f32[3];

    return _mm_setr_ps(
        W1 * X2 + X1 * W2 + Y1 * Z2 - Z1 * Y2,
        W1 * Y2 - X1 * Z2 + Y1 * W2 + Z1 * X2,
        W1 * Z2 + X1 * Y2 - Y1 * X2 + Z1 * W2,
        W1 * W2 - X1 * X2 - Y1 * Y2 - Z1 * Z2);
}

struct alignas(16) FTransform
{
public:
    static const FTransform Identity;

    FORCEINLINE FTransform()
        : Rotation(_mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f))
        , Translation(_mm_setzero_ps())
        , Scale3D(_mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f))
    {
    }

    FORCEINLINE explicit FTransform(const FRotator& InRotation, const FVector& InTranslation, const FVector& InScale3D)
    {
        const FQuat InQuatRotation = InRotation.Quaternion();

        Rotation = _mm_setr_ps(InQuatRotation.X, InQuatRotation.Y, InQuatRotation.Z, InQuatRotation.W);
        Translation = _mm_setr_ps(InTranslation.X, InTranslation.Y, InTranslation.Z, 0.0f);
        Scale3D = _mm_setr_ps(InScale3D.X, InScale3D.Y, InScale3D.Z, 0.0f);
    }

    FORCEINLINE explicit FTransform(const FQuat& InRotation, const FVector& InTranslation, const FVector& InScale3D)
        : Rotation(_mm_setr_ps(InRotation.X, InRotation.Y, InRotation.Z, InRotation.W))
        , Translation(_mm_setr_ps(InTranslation.X, InTranslation.Y, InTranslation.Z, 0.0f))
        , Scale3D(_mm_setr_ps(InScale3D.X, InScale3D.Y, InScale3D.Z, 0.0f))
    {
    }

    FORCEINLINE FVector GetTranslation() const
    {
        return FVector(Translation.m128_f32[0], Translation.m128_f32[1], Translation.m128_f32[2]);
    }

    FORCEINLINE FQuat GetRotation() const
    {
        return FQuat(Rotation.m128_f32[0], Rotation.m128_f32[1], Rotation.m128_f32[2], Rotation.m128_f32[3]);
    }

    FORCEINLINE FVector GetScale3D() const
    {
        return FVector(Scale3D.m128_f32[0], Scale3D.m128_f32[1], Scale3D.m128_f32[2]);
    }

    FORCEINLINE void SetLocation(const FVector& Origin)
    {
        Translation = _mm_setr_ps(Origin.X, Origin.Y, Origin.Z, 0.0f);
    }

    FORCEINLINE void SetRotation(const FQuat& NewRotation)
    {
        Rotation = _mm_setr_ps(NewRotation.X, NewRotation.Y, NewRotation.Z, NewRotation.W);
    }

    FORCEINLINE FVector TransformPosition(const FVector& V) const
    {
        const VectorRegister InputVectorW0 = _mm_setr_ps(V.X, V.Y, V.Z, 0.0f);
        const VectorRegister ScaledVec = _mm_mul_ps(Scale3D, InputVectorW0);
        const VectorRegister RotatedVec = VectorQuaternionRotateVector(Rotation, ScaledVec);
        const VectorRegister TranslatedVec = _mm_add_ps(RotatedVec, Translation);

        return FVector(TranslatedVec.m128_f32[0], TranslatedVec.m128_f32[1], TranslatedVec.m128_f32[2]);
    }

    static void Multiply(FTransform* OutTransform, const FTransform* A, const FTransform* B)
    {
        const VectorRegister MinScale = _mm_min_ps(A->Scale3D, B->Scale3D);

        if (_mm_movemask_ps(_mm_cmplt_ps(MinScale, _mm_setzero_ps())))
        {
            MultiplyUsingMatrixWithScale(OutTransform, A, B);
            return;
        }

        const VectorRegister QuatA = A->Rotation;
        const VectorRegister QuatB = B->Rotation;
        const VectorRegister TranslateA = A->Translation;
        const VectorRegister TranslateB = B->Translation;
        const VectorRegister ScaleA = A->Scale3D;
        const VectorRegister ScaleB = B->Scale3D;

        OutTransform->Rotation = VectorQuaternionMultiply2(QuatB, QuatA);

        const VectorRegister ScaledTransA = _mm_mul_ps(TranslateA, ScaleB);
        const VectorRegister RotatedTranslate = VectorQuaternionRotateVector(QuatB, ScaledTransA);

        OutTransform->Translation = _mm_add_ps(RotatedTranslate, TranslateB);
        OutTransform->Scale3D = _mm_mul_ps(ScaleA, ScaleB);
    }

protected:
    void ToMatrixWithScale(float (&M)[4][4]) const
    {
        const float X = Rotation.m128_f32[0], Y = Rotation.m128_f32[1], Z = Rotation.m128_f32[2], W = Rotation.m128_f32[3];
        const float SX = Scale3D.m128_f32[0], SY = Scale3D.m128_f32[1], SZ = Scale3D.m128_f32[2];
        const float X2 = X + X, Y2 = Y + Y, Z2 = Z + Z;
        const float XX2 = X * X2, YY2 = Y * Y2, ZZ2 = Z * Z2;
        const float YZ2 = Y * Z2, WX2 = W * X2;
        const float XY2 = X * Y2, WZ2 = W * Z2;
        const float XZ2 = X * Z2, WY2 = W * Y2;

        M[3][0] = Translation.m128_f32[0];
        M[3][1] = Translation.m128_f32[1];
        M[3][2] = Translation.m128_f32[2];
        M[0][0] = (1.0f - (YY2 + ZZ2)) * SX;
        M[1][1] = (1.0f - (XX2 + ZZ2)) * SY;
        M[2][2] = (1.0f - (XX2 + YY2)) * SZ;
        M[2][1] = (YZ2 - WX2) * SZ;
        M[1][2] = (YZ2 + WX2) * SY;
        M[1][0] = (XY2 - WZ2) * SY;
        M[0][1] = (XY2 + WZ2) * SX;
        M[2][0] = (XZ2 + WY2) * SZ;
        M[0][2] = (XZ2 - WY2) * SX;
        M[0][3] = 0.0f;
        M[1][3] = 0.0f;
        M[2][3] = 0.0f;
        M[3][3] = 1.0f;
    }

    static void MultiplyUsingMatrixWithScale(FTransform* OutTransform, const FTransform* A, const FTransform* B)
    {
        float AMatrix[4][4];
        float BMatrix[4][4];
        float M[4][4];

        A->ToMatrixWithScale(AMatrix);
        B->ToMatrixWithScale(BMatrix);

        for (int32 Row = 0; Row < 4; Row++)
        {
            for (int32 Column = 0; Column < 4; Column++)
                M[Row][Column] = AMatrix[Row][0] * BMatrix[0][Column] + AMatrix[Row][1] * BMatrix[1][Column] + AMatrix[Row][2] * BMatrix[2][Column] + AMatrix[Row][3] * BMatrix[3][Column];
        }

        const VectorRegister DesiredScale = _mm_mul_ps(A->Scale3D, B->Scale3D);

        for (int32 Axis = 0; Axis < 3; Axis++)
        {
            const float SquareSum = M[Axis][0] * M[Axis][0] + M[Axis][1] * M[Axis][1] + M[Axis][2] * M[Axis][2];
            const float Scale = SquareSum - 1.e-8f >= 0.0f ? FMath::InvSqrt(SquareSum) : 1.0f;
            const float SignedScale = DesiredScale.m128_f32[Axis] >= 0.0f ? 1.0f : -1.0f;

            M[Axis][0] *= Scale * SignedScale;
            M[Axis][1] *= Scale * SignedScale;
            M[Axis][2] *= Scale * SignedScale;
        }

        float Quat[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        bool bZeroAxis = false;

        for (int32 Axis = 0; Axis < 3; Axis++)
        {
            if (fabsf(M[Axis][0]) <= 1.e-4f && fabsf(M[Axis][1]) <= 1.e-4f && fabsf(M[Axis][2]) <= 1.e-4f)
                bZeroAxis = true;
        }

        if (!bZeroAxis)
        {
            const float Trace = M[0][0] + M[1][1] + M[2][2];

            if (Trace > 0.0f)
            {
                const float InvS = FMath::InvSqrt(Trace + 1.0f);
                const float S = 0.5f * InvS;

                Quat[3] = 0.5f * (1.0f / InvS);
                Quat[0] = (M[1][2] - M[2][1]) * S;
                Quat[1] = (M[2][0] - M[0][2]) * S;
                Quat[2] = (M[0][1] - M[1][0]) * S;
            }
            else
            {
                int32 I = 0;

                if (M[1][1] > M[0][0])
                    I = 1;

                if (M[2][2] > M[I][I])
                    I = 2;

                const int32 Next[3] = { 1, 2, 0 };
                const int32 J = Next[I];
                const int32 K = Next[J];
                const float InvS = FMath::InvSqrt(M[I][I] - M[J][J] - M[K][K] + 1.0f);
                const float S = 0.5f * InvS;

                Quat[I] = 0.5f * (1.0f / InvS);
                Quat[3] = (M[J][K] - M[K][J]) * S;
                Quat[J] = (M[I][J] + M[J][I]) * S;
                Quat[K] = (M[I][K] + M[K][I]) * S;
            }

            const float SquareSum = Quat[0] * Quat[0] + Quat[1] * Quat[1] + Quat[2] * Quat[2] + Quat[3] * Quat[3];

            if (SquareSum >= 1.e-8f)
            {
                const float Scale = FMath::InvSqrt(SquareSum);

                for (auto& Component : Quat)
                    Component *= Scale;
            }
            else
            {
                Quat[0] = 0.0f;
                Quat[1] = 0.0f;
                Quat[2] = 0.0f;
                Quat[3] = 1.0f;
            }
        }

        OutTransform->Scale3D = DesiredScale;
        OutTransform->Rotation = _mm_setr_ps(Quat[0], Quat[1], Quat[2], Quat[3]);
        OutTransform->Translation = _mm_setr_ps(M[3][0], M[3][1], M[3][2], 0.0f);
    }

    VectorRegister Rotation;
    VectorRegister Translation;
    VectorRegister Scale3D;
};

namespace UE::Math
{
    template<typename T>
    struct TTransform
    {
        TQuat<T> Rotation;
        TVector<T> Translation;
        T TranslationW;
        TVector<T> Scale3D;
        T Scale3DW;

        TTransform() = default;

        FORCEINLINE explicit TTransform(const FTransform& Transform)
            : Rotation(Transform.GetRotation())
            , Translation(Transform.GetTranslation())
            , TranslationW(0)
            , Scale3D(Transform.GetScale3D())
            , Scale3DW(0)
        {
        }

        FORCEINLINE explicit operator FTransform() const
        {
            return FTransform((FQuat)Rotation, (FVector)Translation, (FVector)Scale3D);
        }
    };
}

using FTransform3d = UE::Math::TTransform<double>;

template<>
struct TGameLayout<FTransform> : TLWCGameLayout<FTransform, FTransform3d>
{
};

