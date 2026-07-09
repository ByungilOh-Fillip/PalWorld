#include "PW_ST_EventsTags.h"

namespace PW_ST_EventsTags
{
    // 이벤트 태그
    UE_DEFINE_GAMEPLAY_TAG(Event_SenseThreat, "Event.SenseThreat");

    // 상태(State) 애니메이션 동기화 태그
    UE_DEFINE_GAMEPLAY_TAG(State_Peaceful_Wander, "State.Peaceful.Wander");
    UE_DEFINE_GAMEPLAY_TAG(State_Peaceful_Rest, "State.Peaceful.Rest");

    UE_DEFINE_GAMEPLAY_TAG(State_Combat_Circle, "State.Combat.Circle");
    UE_DEFINE_GAMEPLAY_TAG(State_Combat_Skill, "State.Combat.Skill");
    UE_DEFINE_GAMEPLAY_TAG(State_Combat_Kite, "State.Combat.Kite");

    UE_DEFINE_GAMEPLAY_TAG(State_Work_Sleep, "State.Work.Sleep");
    UE_DEFINE_GAMEPLAY_TAG(State_Work_Eat, "State.Work.Eat");
    UE_DEFINE_GAMEPLAY_TAG(State_Work_DoWork, "State.Work.DoWork");

    UE_DEFINE_GAMEPLAY_TAG(State_Runaway, "State.Runaway");
}
