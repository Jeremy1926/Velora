#include "pch.h"
#include "FortAthenaMatchmakingWidget.h"
#include "Scanner.h"
#include "Detour.h"

void UFortAthenaMatchmakingWidgetLegacy::HandleActivityQueryComplete(TSharedPtr<FFortActivity> Activity)
{
    DETOUR(UFortAthenaMatchmakingWidgetLegacy, HandleActivityQueryComplete, Anchor::Hook()
        .String(TEXT("UFortAthenaMatchmakingWidgetLegacy::HandleActivityQueryComplete"))
        .Start()
        .Has("cmp [r12], rdi; mov r*, [rax+0x*]; setnz al")
        .Optional()
        .Inline());

    if (!GetOwningLocalPlayer())
        return;

    Original(MoveTemp(Activity));
}
