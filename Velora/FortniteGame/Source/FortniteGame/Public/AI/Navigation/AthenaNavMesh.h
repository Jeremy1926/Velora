#pragma once

#include "AI/Navigation/FortNavMesh.h"

class AAthenaNavMesh : public AFortNavMesh
{
public:
    static UClass* StaticClass();

    void RebuildAll();
};
