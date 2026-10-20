#include "pch.h"
#include "Scanner.h"

bool& GIsAllowingParticles = *Anchor::Data<bool*>(TEXT("GIsAllowingParticles"))
    .String(TEXT("CreateRenderState_Concurrent @ %fs %s"))
    .Last("cmp byte ptr [0x*], 0x6")
    .Find("cmp *[0x*], *")
    .Target()
    .Get();
