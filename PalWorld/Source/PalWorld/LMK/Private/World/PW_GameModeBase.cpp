#include "World/PW_GameModeBase.h"

#include "World/PW_WorldGameState.h"

APW_GameModeBase::APW_GameModeBase()
{
	GameStateClass = APW_WorldGameState::StaticClass();
}
