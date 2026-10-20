#include "pch.h"
#include "Logging/LogMacros.h"
#include "Scanner.h"

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

decltype(FMsg::Logf_Internal) FMsg::Logf_Internal = Anchor::Function<decltype(FMsg::Logf_Internal)>(TEXT("FMsg::Logf_Internal"))
    .String(TEXT("AddClientConnection: Added client connection: %s"))
    .Call()
    .Get();
