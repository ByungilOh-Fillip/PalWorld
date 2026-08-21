#pragma once

#include "NativeGameplayTags.h"

namespace PW_ST_EventsTags
{
    // 이벤트 태그
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_SenseThreat);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Hit);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_TargetLost);

    // 상태(State) 애니메이션 동기화 태그
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Peaceful_Wander);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Peaceful_Rest);
    
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Circle);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Skill);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Kite);
    
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Work_Sleep);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Work_Eat);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Work_DoWork);
    
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Runaway);
};
