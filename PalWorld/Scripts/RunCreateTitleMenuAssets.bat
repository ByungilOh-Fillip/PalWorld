@echo off
setlocal

set "PROJECT=%~dp0..\PalWorld.uproject"
set "SCRIPT=%~dp0CreateTitleMenuAssets.py"

if not defined UE_EDITOR_CMD (
  echo Set UE_EDITOR_CMD to your UnrealEditor-Cmd.exe path first.
  echo Example:
  echo   set UE_EDITOR_CMD=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe
  exit /b 1
)

"%UE_EDITOR_CMD%" "%PROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4
