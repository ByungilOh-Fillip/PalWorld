import json
import os

import unreal


OUTPUT_PATH = r"C:\workspace\Github\PalWorld\PalWorld\Content\Python\EBS_ReferenceDump.json"

ASSETS = [
    "/Game/EasyBuildingSystem/Blueprints/BuildingObjects/Modular/BP_EBS_Building_Foundation.BP_EBS_Building_Foundation",
    "/Game/EasyBuildingSystem/Blueprints/BuildingObjects/Modular/BP_EBS_Building_Wall.BP_EBS_Building_Wall",
    "/Game/EasyBuildingSystem/Blueprints/BuildingObjects/Modular/BP_EBS_Building_Roof.BP_EBS_Building_Roof",
    "/Game/EasyBuildingSystem/Blueprints/BuildingObjects/Base/BP_EBS_Building_BaseObject.BP_EBS_Building_BaseObject",
]


def vector_to_dict(value):
    return {
        "x": float(value.x),
        "y": float(value.y),
        "z": float(value.z),
    }


def rotator_to_dict(value):
    return {
        "pitch": float(value.pitch),
        "yaw": float(value.yaw),
        "roll": float(value.roll),
    }


def transform_to_dict(component):
    try:
        relative_location = component.get_relative_location()
        relative_rotation = component.get_relative_rotation()
        relative_scale = component.get_relative_scale3d()
    except Exception:
        return {}

    return {
        "relative_location": vector_to_dict(relative_location),
        "relative_rotation": rotator_to_dict(relative_rotation),
        "relative_scale": vector_to_dict(relative_scale),
    }


def static_mesh_to_dict(static_mesh):
    if static_mesh is None:
        return None

    bounds = static_mesh.get_bounds()
    return {
        "name": static_mesh.get_name(),
        "path": static_mesh.get_path_name(),
        "origin": vector_to_dict(bounds.origin),
        "box_extent": vector_to_dict(bounds.box_extent),
        "sphere_radius": float(bounds.sphere_radius),
    }


def component_to_dict(component):
    data = {
        "name": component.get_name(),
        "class": component.get_class().get_name(),
        "transform": transform_to_dict(component),
    }

    try:
        parent = component.get_attach_parent()
        data["attach_parent"] = parent.get_name() if parent else None
    except Exception:
        data["attach_parent"] = None

    if isinstance(component, unreal.StaticMeshComponent):
        try:
            data["static_mesh"] = static_mesh_to_dict(component.static_mesh)
        except Exception:
            data["static_mesh"] = None

    return data


def get_blueprint_components(asset_path):
    blueprint_asset = unreal.load_asset(asset_path)
    generated_class = unreal.load_class(None, f"{asset_path}_C")
    if generated_class is None:
        return None

    cdo = unreal.get_default_object(generated_class)
    if cdo is None:
        return None

    result = {
        "asset_path": asset_path,
        "class": generated_class.get_name(),
        "cdo": cdo.get_name(),
        "components": [],
        "properties": {},
    }

    try:
        for component in cdo.get_components_by_class(unreal.SceneComponent):
            result["components"].append(component_to_dict(component))
    except Exception as error:
        result["component_error"] = str(error)

    try:
        scs = blueprint_asset.get_editor_property("simple_construction_script") if blueprint_asset else None
        nodes = scs.get_all_nodes() if scs else []
        result["scs_components"] = []
        for node in nodes:
            try:
                template = node.get_editor_property("component_template")
            except Exception:
                template = None

            if template is None:
                continue

            template_data = component_to_dict(template)
            try:
                template_data["variable_name"] = str(node.get_variable_name())
            except Exception:
                try:
                    template_data["variable_name"] = str(node.get_editor_property("variable_name"))
                except Exception:
                    template_data["variable_name"] = ""
            result["scs_components"].append(template_data)
    except Exception as error:
        result["scs_error"] = str(error)

    try:
        subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        handles = subsystem.k2_gather_subobject_data_for_blueprint(blueprint_asset) if subsystem and blueprint_asset else []
        result["subobject_components"] = []
        result["subobject_debug"] = {
            "subsystem_methods": [name for name in dir(subsystem) if "subobject" in name.lower() or "data" in name.lower()],
            "handle_count": len(handles),
        }
        for handle in handles:
            data = subsystem.k2_find_subobject_data_from_handle(handle)
            data_methods = [name for name in dir(data) if not name.startswith("_")]
            data_dict = {}
            try:
                data_dict = data.to_dict()
            except Exception:
                data_dict = {}
            template = None
            for value in data_dict.values():
                if isinstance(value, unreal.Object):
                    template = value
                    break
            for method_name in [
                "get_object",
                "get_component_template",
                "get_object_for_blueprint",
            ]:
                if hasattr(data, method_name):
                    try:
                        template = getattr(data, method_name)()
                        if template is not None:
                            break
                    except Exception:
                        pass

            entry = {
                "data_methods": data_methods,
                "data_dict": {key: str(value) for key, value in data_dict.items()},
                "object": template.get_name() if template else None,
                "object_class": template.get_class().get_name() if template else None,
            }
            if isinstance(template, unreal.SceneComponent):
                entry.update(component_to_dict(template))
            result["subobject_components"].append(entry)
    except Exception as error:
        result["subobject_error"] = str(error)

    for property_name in [
        "building_type",
        "socket_name",
        "is_floor",
        "placed_on_landscape",
        "rotate_angle",
        "snap_settings",
        "manual_input_rotation",
    ]:
        try:
            value = cdo.get_editor_property(property_name)
            result["properties"][property_name] = str(value)
        except Exception:
            pass

    return result


def main():
    dump = {}
    for asset_path in ASSETS:
        dump[asset_path] = get_blueprint_components(asset_path)

    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    with open(OUTPUT_PATH, "w", encoding="utf-8") as output_file:
        json.dump(dump, output_file, indent=2, ensure_ascii=False)

    unreal.log(f"[PWExtractEBSReference] Wrote {OUTPUT_PATH}")


main()
