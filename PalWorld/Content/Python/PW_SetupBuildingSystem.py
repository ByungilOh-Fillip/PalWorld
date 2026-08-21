import os

import unreal


BUILDING_DIR = "/Game/_Private/LMK/BluePrints/Building"
BUILDING_UI_DIR = "/Game/_Private/LMK/UI/Building"

ASSETS = {
    "foundation_bp": f"{BUILDING_DIR}/BP_PW_Building_Foundation.BP_PW_Building_Foundation",
    "wall_bp": f"{BUILDING_DIR}/BP_PW_Building_Wall.BP_PW_Building_Wall",
    "roof_bp": f"{BUILDING_DIR}/BP_PW_Building_Roof.BP_PW_Building_Roof",
    "radial_wbp_class": f"{BUILDING_DIR}/WBP_BuildingRadialMenu.WBP_BuildingRadialMenu_C",
    "player_bp": "/Game/PJH/Blueprints/BP_PlayerCharacter.BP_PlayerCharacter",
    "controller_bp": "/Game/PJH/Blueprints/BP_PlayerController.BP_PlayerController",
    "wood_foundation_mesh": "/Game/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Foundation.SM_Stylized_Wood_Foundation",
    "wood_wall_mesh": "/Game/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Wall.SM_Stylized_Wood_Wall",
    "wood_roof_mesh": "/Game/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Roof.SM_Stylized_Wood_Roof",
    "mat_wood": "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Wood.MI_Stylized_Structures_Wood",
    "mat_stone": "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Stone.MI_Stylized_Structures_Stone",
    "mat_metal": "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Metal.MI_Stylized_Structures_Metal",
    "mat_wood_roof": "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Wood_Roof.MI_Stylized_Structures_Wood_Roof",
    "mat_stone_roof": "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Stone_Roof.MI_Stylized_Structures_Stone_Roof",
    "preview_valid": "/Game/EasyBuildingSystem/Materials/Instances/Dummy/MI_Can_Build.MI_Can_Build",
    "preview_invalid": "/Game/EasyBuildingSystem/Materials/Instances/Dummy/MI_CanNot_Build.MI_CanNot_Build",
    "radial_menu_png": "T_BuildingRadialMenuBase.png",
}

PLAYER_BP_CANDIDATES = [
    "/Game/PJH/Blueprints/BP_PlayerCharacter.BP_PlayerCharacter",
    "/Game/_Private/LMK/BluePrints/BP_PlayerCharacter_Test_LMK.BP_PlayerCharacter_Test_LMK",
]

CONTROLLER_BP_CANDIDATES = [
    "/Game/PJH/Blueprints/BP_PlayerController.BP_PlayerController",
]


def get_asset_class_path(asset_data):
    try:
        return str(asset_data.asset_class_path.asset_name)
    except Exception:
        try:
            return str(asset_data.asset_class)
        except Exception:
            return ""


def get_asset_object_path(asset_data):
    try:
        return str(asset_data.object_path)
    except Exception:
        package_name = str(asset_data.package_name)
        asset_name = str(asset_data.asset_name)
        return f"{package_name}.{asset_name}"


def load_asset(path):
    asset = unreal.load_asset(path)
    if asset is None:
        unreal.log_warning(f"[PWBuildingSetup] Missing asset: {path}")
    return asset


def load_class(path):
    cls = unreal.load_class(None, path)
    if cls is None:
        unreal.log_warning(f"[PWBuildingSetup] Missing class: {path}")
    return cls


def enum_value(enum_cls, name):
    return getattr(enum_cls, name)


def make_material_profile(material_type, display_name, material, durability):
    profile = unreal.PW_BuildingMaterialProfile()
    profile.set_editor_property("material_type", material_type)
    profile.set_editor_property("display_name", unreal.Text(display_name))
    profile.set_editor_property("material_override", material)
    profile.set_editor_property("max_durability", float(durability))
    return profile


def make_transform(location, yaw=0.0):
    return unreal.Transform(
        location=unreal.Vector(location[0], location[1], location[2]),
        rotation=unreal.Rotator(0.0, yaw, 0.0),
        scale=unreal.Vector(1.0, 1.0, 1.0),
    )


def make_snap_point(snap_id, accepts_piece_type, location, yaw=0.0, radius=220.0, child_location=(0.0, 0.0, 0.0), child_yaw=0.0):
    snap = unreal.PW_BuildingSnapPoint()
    snap.set_editor_property("snap_id", snap_id)
    snap.set_editor_property("accepts_piece_type", accepts_piece_type)
    snap.set_editor_property("local_transform", make_transform(location, yaw))
    snap.set_editor_property("child_local_transform", make_transform(child_location, child_yaw))
    snap.set_editor_property("snap_radius", float(radius))
    return snap


def make_piece_definition(piece_type, display_name, piece_class, valid_material, invalid_material, overlap_extent, rotation_step_degrees):
    definition = unreal.PW_BuildingPieceDefinition()
    definition.set_editor_property("piece_type", piece_type)
    definition.set_editor_property("display_name", unreal.Text(display_name))
    definition.set_editor_property("piece_class", piece_class)
    definition.set_editor_property("rotation_step_degrees", float(rotation_step_degrees))
    definition.set_editor_property("preview_valid_material", valid_material)
    definition.set_editor_property("preview_invalid_material", invalid_material)
    definition.set_editor_property("placement_overlap_extent", unreal.Vector(overlap_extent[0], overlap_extent[1], overlap_extent[2]))
    return definition


def get_blueprint_cdo(asset_path):
    generated_class = load_class(f"{asset_path}_C")
    if generated_class is None:
        return None
    return unreal.get_default_object(generated_class)


def get_blueprint_asset(asset_path):
    return load_asset(asset_path)


def save_blueprint_asset(asset_path):
    package_path = asset_path.split(".")[0]
    unreal.EditorAssetLibrary.save_asset(package_path)


def import_radial_menu_texture():
    source_path = unreal.Paths.project_content_dir() + "_Private/LMK/UI/Building/" + ASSETS["radial_menu_png"]
    if not os.path.exists(source_path):
        unreal.log_warning(f"[PWBuildingSetup] Missing radial menu PNG source: {source_path}")
        return None

    destination_asset = f"{BUILDING_UI_DIR}/T_BuildingRadialMenuBase"
    existing_texture = unreal.load_asset(destination_asset)
    if existing_texture is not None:
        unreal.log(f"[PWBuildingSetup] Radial menu texture already exists: {destination_asset}")
        return existing_texture

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_path)
    task.set_editor_property("destination_path", BUILDING_UI_DIR)
    task.set_editor_property("destination_name", "T_BuildingRadialMenuBase")
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    task.set_editor_property("replace_existing", True)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported_texture = unreal.load_asset(destination_asset)
    if imported_texture:
        unreal.log(f"[PWBuildingSetup] Imported radial menu texture: {destination_asset}")
    else:
        unreal.log_warning(f"[PWBuildingSetup] Failed to import radial menu texture: {source_path}")
    return imported_texture


def find_component(cdo, class_name, instance_names=None, log_missing=True):
    if cdo is None:
        return None
    if not hasattr(cdo, "get_components_by_class"):
        if log_missing:
            unreal.log_warning(f"[PWBuildingSetup] Object {cdo.get_name()} cannot own actor components.")
        return None

    instance_names = instance_names or []
    target_class = getattr(unreal, class_name, None)
    component_names = []
    for component in cdo.get_components_by_class(unreal.ActorComponent):
        component_name = component.get_name()
        component_class = component.get_class()
        component_class_name = component_class.get_name()
        component_names.append(f"{component_name}:{component_class_name}")
        if component_name in instance_names:
            return component
        if component_class_name == class_name or class_name in component_class_name:
            return component
        if target_class is not None:
            try:
                if component_class.is_child_of(target_class):
                    return component
            except Exception:
                pass
    if log_missing:
        unreal.log_warning(f"[PWBuildingSetup] Could not find component {class_name}. Existing components: {component_names}")
    return None


def find_scs_component_template(asset_path, class_name, instance_names=None):
    bp = get_blueprint_asset(asset_path)
    if bp is None:
        return None

    instance_names = instance_names or []
    target_class = getattr(unreal, class_name, None)

    try:
        scs = bp.get_editor_property("simple_construction_script")
    except Exception:
        scs = None

    if scs is None:
        return None

    try:
        nodes = scs.get_all_nodes()
    except Exception:
        nodes = []

    for node in nodes:
        try:
            variable_name = str(node.get_variable_name())
        except Exception:
            try:
                variable_name = str(node.get_editor_property("variable_name"))
            except Exception:
                variable_name = ""

        try:
            component_template = node.get_editor_property("component_template")
        except Exception:
            component_template = None

        if component_template is None:
            continue

        component_class = component_template.get_class()
        component_class_name = component_class.get_name()

        if variable_name in instance_names or component_template.get_name() in instance_names:
            return component_template

        if component_class_name == class_name or class_name in component_class_name:
            return component_template

        if target_class is not None:
            try:
                if component_class.is_child_of(target_class):
                    return component_template
            except Exception:
                pass

    return None


def find_component_in_candidates(candidate_paths, class_name, instance_names=None):
    for object_path in candidate_paths:
        cdo = get_blueprint_cdo(object_path)
        component = find_component(cdo, class_name, instance_names, log_missing=False)
        if component is not None:
            unreal.log(f"[PWBuildingSetup] Using {object_path} for {class_name}.")
            return object_path, cdo, component

        component_template = find_scs_component_template(object_path, class_name, instance_names)
        if component_template is not None:
            unreal.log(f"[PWBuildingSetup] Using SCS component on {object_path} for {class_name}.")
            return object_path, cdo, component_template

    unreal.log_warning(
        f"[PWBuildingSetup] Could not find {class_name} in explicit candidates: {candidate_paths}. "
        "Open the actual BP class, add the component to Components panel, Compile/Save, "
        "or add that BP object path to PLAYER_BP_CANDIDATES / CONTROLLER_BP_CANDIDATES."
    )
    return None, None, None


def configure_building_piece(bp_path, mesh_path, piece_type, material_profiles, snap_points):
    cdo = get_blueprint_cdo(bp_path)
    if cdo is None:
        return None

    mesh_component = cdo.get_editor_property("mesh_component")
    mesh = load_asset(mesh_path)
    if mesh_component and mesh:
        mesh_component.set_static_mesh(mesh)
        if material_profiles and material_profiles[0].get_editor_property("material_override"):
            mesh_component.set_material(0, material_profiles[0].get_editor_property("material_override"))
        try:
            mesh_component.set_can_ever_affect_navigation(True)
        except Exception:
            unreal.log(f"[PWBuildingSetup] Skipped Python CanEverAffectNavigation setup on {bp_path}; APW_BuildingPieceActor sets it in C++.")

    cdo.set_editor_property("default_piece_type", piece_type)
    cdo.set_editor_property("piece_type", piece_type)
    cdo.set_editor_property("material_profiles", material_profiles)
    cdo.set_editor_property("snap_points", snap_points)
    save_blueprint_asset(bp_path)
    unreal.log(f"[PWBuildingSetup] Configured {bp_path}")
    return cdo


def configure_player_components():
    unreal.log(
        "[PWBuildingSetup] Player/controller component template setup skipped. "
        "PW_PlayerBuildingPlacement and PW_BuildingInput use C++ default fallbacks."
    )


def main():
    import_radial_menu_texture()

    wood = load_asset(ASSETS["mat_wood"])
    stone = load_asset(ASSETS["mat_stone"])
    metal = load_asset(ASSETS["mat_metal"])
    wood_roof = load_asset(ASSETS["mat_wood_roof"])
    stone_roof = load_asset(ASSETS["mat_stone_roof"])

    wood_type = unreal.PW_BuildingMaterialType.WOOD
    stone_type = unreal.PW_BuildingMaterialType.STONE
    iron_type = unreal.PW_BuildingMaterialType.IRON

    foundation_profiles = [
        make_material_profile(wood_type, "Wood", wood, 500.0),
        make_material_profile(stone_type, "Stone", stone, 1000.0),
        make_material_profile(iron_type, "Iron", metal, 2000.0),
    ]
    wall_profiles = foundation_profiles
    roof_profiles = [
        make_material_profile(wood_type, "Wood", wood_roof or wood, 500.0),
        make_material_profile(stone_type, "Stone", stone_roof or stone, 1000.0),
        make_material_profile(iron_type, "Iron", metal, 2000.0),
    ]

    wall_type = unreal.PW_BuildingPieceType.WALL
    roof_type = unreal.PW_BuildingPieceType.ROOF
    foundation_type = unreal.PW_BuildingPieceType.FOUNDATION

    foundation_snaps = [
        make_snap_point("Foundation_North", foundation_type, (0.0, 200.0, 0.0), 0.0, 160.0, (0.0, -200.0, 0.0), 0.0),
        make_snap_point("Foundation_East", foundation_type, (200.0, 0.0, 0.0), 90.0, 160.0, (-200.0, 0.0, 0.0), 0.0),
        make_snap_point("Foundation_South", foundation_type, (0.0, -200.0, 0.0), 180.0, 160.0, (0.0, 200.0, 0.0), 0.0),
        make_snap_point("Foundation_West", foundation_type, (-200.0, 0.0, 0.0), -90.0, 160.0, (200.0, 0.0, 0.0), 0.0),
        make_snap_point("Wall_North", wall_type, (0.0, 200.0, 170.0), 0.0, 220.0),
        make_snap_point("Wall_East", wall_type, (200.0, 0.0, 170.0), 90.0, 220.0),
        make_snap_point("Wall_South", wall_type, (0.0, -200.0, 170.0), 180.0, 220.0),
        make_snap_point("Wall_West", wall_type, (-200.0, 0.0, 170.0), -90.0, 220.0),
    ]
    wall_snaps = [
        make_snap_point("Roof_Top", roof_type, (0.0, 0.0, 340.0), 0.0, 220.0),
    ]

    configure_building_piece(ASSETS["foundation_bp"], ASSETS["wood_foundation_mesh"], unreal.PW_BuildingPieceType.FOUNDATION, foundation_profiles, foundation_snaps)
    configure_building_piece(ASSETS["wall_bp"], ASSETS["wood_wall_mesh"], unreal.PW_BuildingPieceType.WALL, wall_profiles, wall_snaps)
    configure_building_piece(ASSETS["roof_bp"], ASSETS["wood_roof_mesh"], unreal.PW_BuildingPieceType.ROOF, roof_profiles, [])
    configure_player_components()
    unreal.log("[PWBuildingSetup] Done.")


main()
