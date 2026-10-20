#pragma once

#include "MinHook.h"
#include "Scanner.h"

extern "C"
{
#include "buffer.h"
#include "trampoline.h"
}

using Anchor::Detour;
using Anchor::DetourCall;
using Anchor::DetourVTable;
using Anchor::PatchBranch;
using Anchor::Patch;
