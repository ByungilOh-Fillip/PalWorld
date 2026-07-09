#include "Menu/PW_MenuGameMode.h"

#include "Menu/PW_MenuPlayerController.h"

APW_MenuGameMode::APW_MenuGameMode()
{
	PlayerControllerClass = APW_MenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}
