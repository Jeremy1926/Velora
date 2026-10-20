#pragma once

#include "HAL/Platform.h"

namespace ECollisionShape
{
    enum Type
    {
        Line,
        Box,
        Sphere,
        Capsule
    };
};

struct FCollisionShape
{
    ECollisionShape::Type ShapeType;

    union
    {
        struct
        {
            float HalfExtentX;
            float HalfExtentY;
            float HalfExtentZ;
        } Box;

        struct
        {
            float Radius;
        } Sphere;

        struct
        {
            float Radius;
            float HalfHeight;
        } Capsule;
    };

    FCollisionShape()
    {
        ShapeType = ECollisionShape::Line;
    }

    void SetSphere(const float Radius)
    {
        ShapeType = ECollisionShape::Sphere;
        Sphere.Radius = Radius;
    }

    static FCollisionShape MakeSphere(const float SphereRadius)
    {
        FCollisionShape SphereShape;
        SphereShape.SetSphere(SphereRadius);
        return SphereShape;
    }
};
