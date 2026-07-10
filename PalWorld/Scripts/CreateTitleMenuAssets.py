import unreal


UI_DIR = "/Game/_Private/LMK/UI/Menu"
MENU_DIR = "/Game/_Private/LMK/Menu"
SOURCE_IMAGE = r"D:\Github\PalWorld\PalWorld\Content\_Private\LMK\UI\Menu\T_MainMenu_Background_Source.png"


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def load_class(path):
    cls = unreal.load_class(None, path)
    if cls is None:
        raise RuntimeError("Missing class: {}".format(path))
    return cls


def import_background_texture():
    destination_path = "{}/T_MainMenu_Background".format(UI_DIR)
    if unreal.EditorAssetLibrary.does_asset_exist(destination_path):
        return unreal.EditorAssetLibrary.load_asset(destination_path)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", SOURCE_IMAGE)
    task.set_editor_property("destination_path", UI_DIR)
    task.set_editor_property("destination_name", "T_MainMenu_Background")
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    task.set_editor_property("replace_existing", False)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture = unreal.EditorAssetLibrary.load_asset(destination_path)
    if texture is None:
        raise RuntimeError("Failed to import {}".format(SOURCE_IMAGE))
    return texture


def create_widget_blueprint(asset_name, parent_class):
    asset_path = "{}/{}".format(UI_DIR, asset_name)
    existing = unreal.EditorAssetLibrary.load_asset(asset_path)
    if existing is not None:
        return existing

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, UI_DIR, unreal.WidgetBlueprint, factory)
    if asset is None:
        raise RuntimeError("Failed to create {}".format(asset_path))
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    unreal.log("CreateTitleMenuAssets: created {}".format(asset_path))
    return asset


def create_class_blueprint(asset_name, parent_class):
    asset_path = "{}/{}".format(MENU_DIR, asset_name)
    existing = unreal.EditorAssetLibrary.load_asset(asset_path)
    if existing is not None:
        return existing

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, MENU_DIR, unreal.Blueprint, factory)
    if asset is None:
        raise RuntimeError("Failed to create {}".format(asset_path))
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    unreal.log("CreateTitleMenuAssets: created {}".format(asset_path))
    return asset


def create_menu_blueprints():
    pc_parent = load_class("/Script/PalWorld.PW_MenuPlayerController")
    gm_parent = load_class("/Script/PalWorld.PW_MenuGameMode")

    create_class_blueprint("BP_MenuPlayerController", pc_parent)
    create_class_blueprint("BP_MenuGameMode", gm_parent)


def load_blueprint_generated_class(asset_path):
    if hasattr(unreal.EditorAssetLibrary, "load_blueprint_class"):
        cls = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
        if cls is not None:
            return cls

    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError("Missing blueprint asset: {}".format(asset_path))

    cls = asset.get_editor_property("generated_class")
    if cls is None:
        raise RuntimeError("Blueprint has no generated class: {}".format(asset_path))
    return cls


def configure_menu_blueprints():
    main_menu_class = load_blueprint_generated_class("{}/WBP_MainMenu".format(UI_DIR))
    player_controller_class = load_blueprint_generated_class("{}/BP_MenuPlayerController".format(MENU_DIR))
    game_mode_class = load_blueprint_generated_class("{}/BP_MenuGameMode".format(MENU_DIR))

    player_controller_cdo = unreal.get_default_object(player_controller_class)
    player_controller_cdo.set_editor_property("main_menu_widget_class", main_menu_class)

    game_mode_cdo = unreal.get_default_object(game_mode_class)
    game_mode_cdo.set_editor_property("player_controller_class", player_controller_class)

    unreal.EditorAssetLibrary.save_asset("{}/BP_MenuPlayerController".format(MENU_DIR), only_if_is_dirty=False)
    unreal.EditorAssetLibrary.save_asset("{}/BP_MenuGameMode".format(MENU_DIR), only_if_is_dirty=False)
    unreal.log("CreateTitleMenuAssets: configured BP_MenuPlayerController and BP_MenuGameMode")
    return game_mode_class


def create_menu_map(game_mode_class):
    map_path = "{}/MenuMap".format(MENU_DIR)
    if not unreal.EditorAssetLibrary.does_asset_exist(map_path):
        unreal.EditorLevelLibrary.new_level(map_path)
    else:
        unreal.EditorLevelLibrary.load_level(map_path)

    world = unreal.EditorLevelLibrary.get_editor_world()
    world_settings = world.get_world_settings()
    world_settings.set_editor_property("default_game_mode", game_mode_class)
    unreal.EditorLevelLibrary.save_current_level()


def main():
    unreal.log("CreateTitleMenuAssets: started")
    ensure_dir(UI_DIR)
    ensure_dir(MENU_DIR)
    import_background_texture()

    create_widget_blueprint("WBP_MainMenu", load_class("/Script/PalWorld.PW_MainMenuWidget"))
    create_widget_blueprint("WBP_SessionSlot", load_class("/Script/PalWorld.PW_SessionSlotWidget"))
    create_widget_blueprint("WBP_Options", load_class("/Script/PalWorld.PW_OptionsWidget"))

    create_menu_blueprints()
    game_mode_class = configure_menu_blueprints()
    create_menu_map(game_mode_class)

    unreal.EditorAssetLibrary.save_directory(UI_DIR)
    unreal.EditorAssetLibrary.save_directory(MENU_DIR)
    unreal.log("CreateTitleMenuAssets: finished")


try:
    main()
except Exception as exc:
    unreal.log_error("CreateTitleMenuAssets failed: {}".format(exc))
    raise
