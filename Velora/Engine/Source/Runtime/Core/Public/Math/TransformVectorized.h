#pragma once

#include "HAL/Platform.h"
#include "Math/Vector.h"
#include "Math/Quat.h"
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

    FORCEINLINE FVector GetTranslation() const
    {
        return FVector(Translation.m128_f32[0], Translation.m128_f32[1], Translation.m128_f32[2]);
    }

    FORCEINLINE FQuat GetRotation() const
    {
        return FQuat(Rotation.m128_f32[0], Rotation.m128_f32[1], Rotation.m128_f32[2], Rotation.m128_f32[3]);
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

protected:
    VectorRegister Rotation;
    VectorRegister Translation;
    VectorRegister Scale3D;
};
