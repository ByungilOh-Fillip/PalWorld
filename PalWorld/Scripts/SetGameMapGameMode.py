import unreal


GAME_MAP = "/Game/_Private/LMK/Levels/Palworld"
GAME_MODE_CLASS_PATH = "/Game/_Private/LMK/BluePrints/BP_PWGameMode.BP_PWGameMode_C"
MENU_MAP = "/Game/_Private/LMK/Menu/MenuMap"


def load_class(path):
    cls = unreal.load_class(None, path)
    if cls is None:
        raise RuntimeError("Missing class: {}".format(path))
    return cls


def open_level(map_path):
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if level_editor is not None:
        return level_editor.load_level(map_path)
    return unreal.EditorLevelLibrary.load_level(map_path)


def save_current_level():
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if level_editor is not None:
        return level_editor.save_current_level()
    return unreal.EditorLevelLibrary.save_current_level()


def get_editor_world():
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    if editor is not None:
        return editor.get_editor_world()
    return unreal.EditorLevelLibrary.get_editor_world()


def main():
    unreal.log("SetGameMapGameMode: started")
    game_mode_class = load_class(GAME_MODE_CLASS_PATH)

    open_level(GAME_MAP)
    world = get_editor_world()
    world_settings = world.get_world_settings()
    world_settings.set_editor_property("default_game_mode", game_mode_class)
    save_current_level()
    unreal.log("SetGameMapGameMode: set {} default_game_mode to {}".format(GAME_MAP, GAME_MODE_CLASS_PATH))

    if unreal.EditorAssetLibrary.does_asset_exist(MENU_MAP):
        open_level(MENU_MAP)
        unreal.log("SetGameMapGameMode: returned to {}".format(MENU_MAP))

    unreal.log("SetGameMapGameMode: finished")


try:
    main()
except Exception as exc:
    unreal.log_error("SetGameMapGameMode failed: {}".format(exc))
    raise
