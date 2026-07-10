import os
import unreal

TEXTURE_PACKAGE_PATH = "/Game/PJH/UI/Textures"
MATERIAL_PACKAGE_PATH = "/Game/PJH/UI/Materials"

MATERIAL_NAME = "M_UI_CaptureRing_Procedural"
INSTANCE_NAME = "MI_UI_CaptureRing_Procedural"


def ensure_directory(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def delete_asset_if_exists(asset_path: str) -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)


def make_scalar(material: unreal.Material, name: str, value: float, x: int, y: int):
    expr = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionScalarParameter, x, y)
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", value)
    return expr


def make_vector(material: unreal.Material, name: str, value: unreal.LinearColor, x: int, y: int):
    expr = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter, x, y)
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", value)
    return expr


def make_custom_input(name: str):
    custom_input = unreal.CustomInput()
    custom_input.set_editor_property("input_name", name)
    return custom_input


def build_material() -> unreal.Material:
    ensure_directory(MATERIAL_PACKAGE_PATH)
    material_path = f"{MATERIAL_PACKAGE_PATH}/{MATERIAL_NAME}"
    instance_path = f"{MATERIAL_PACKAGE_PATH}/{INSTANCE_NAME}"
    delete_asset_if_exists(instance_path)
    delete_asset_if_exists(material_path)

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(MATERIAL_NAME, MATERIAL_PACKAGE_PATH, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError("Failed to create capture ring material.")

    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_UI)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)

    uv = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, -900, 40)
    progress = make_scalar(material, "Progress", 1.0, -900, 180)
    chance = make_scalar(material, "Chance", 0.0, -900, 300)
    is_capturing = make_scalar(material, "IsCapturing", 0.0, -900, 420)
    color = make_vector(material, "ProgressColor", unreal.LinearColor(0.05, 1.0, 0.72, 1.0), -900, 560)

    custom = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionCustom, -420, 100)
    custom.set_editor_property("description", "CaptureRingFill")
    custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT4)
    custom.set_editor_property(
        "inputs",
        [
            make_custom_input("UV"),
            make_custom_input("Progress"),
            make_custom_input("Chance"),
            make_custom_input("IsCapturing"),
            make_custom_input("ProgressColor"),
        ],
    )
    custom.set_editor_property(
        "code",
        """
float2 centered = UV - float2(0.5, 0.5);
float radius = length(centered);

// 위쪽 12시 방향에서 시작해서 시계 방향으로 차오르게 만든다.
float angle = atan2(centered.x, -centered.y) / 6.28318530718;
angle = angle < 0.0 ? angle + 1.0 : angle;

float progressMask = step(angle, saturate(Progress));

// PNG 배경 상태에 의존하지 않도록 링 자체를 UV로 직접 그린다.
float outer = 0.465;
float inner = 0.395;
float outerEdge = 1.0 - smoothstep(outer - 0.006, outer + 0.006, radius);
float innerEdge = smoothstep(inner - 0.006, inner + 0.006, radius);
float radialMask = outerEdge * innerEdge;

// 팰월드 UI처럼 끊어진 조각감을 주기 위한 세그먼트 마스크.
float segmentCount = 10.0;
float segmentLocal = frac(angle * segmentCount);
float segmentMask = smoothstep(0.055, 0.080, segmentLocal) * (1.0 - smoothstep(0.920, 0.945, segmentLocal));

float ringMask = radialMask * segmentMask * progressMask;

float pulse = IsCapturing > 0.5 ? (0.86 + 0.14 * sin(Chance * 18.0 + Progress * 24.0)) : 1.0;
float3 finalColor = ProgressColor.rgb * pulse * ringMask;
return float4(finalColor, ringMask);
""",
    )

    unreal.MaterialEditingLibrary.connect_material_expressions(uv, "", custom, "UV")
    unreal.MaterialEditingLibrary.connect_material_expressions(progress, "", custom, "Progress")
    unreal.MaterialEditingLibrary.connect_material_expressions(chance, "", custom, "Chance")
    unreal.MaterialEditingLibrary.connect_material_expressions(is_capturing, "", custom, "IsCapturing")
    unreal.MaterialEditingLibrary.connect_material_expressions(color, "", custom, "ProgressColor")

    rgb_mask = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionComponentMask, -120, 80)
    rgb_mask.set_editor_property("r", True)
    rgb_mask.set_editor_property("g", True)
    rgb_mask.set_editor_property("b", True)
    rgb_mask.set_editor_property("a", False)

    alpha_mask = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionComponentMask, -120, 260)
    alpha_mask.set_editor_property("r", False)
    alpha_mask.set_editor_property("g", False)
    alpha_mask.set_editor_property("b", False)
    alpha_mask.set_editor_property("a", True)

    unreal.MaterialEditingLibrary.connect_material_expressions(custom, "", rgb_mask, "")
    unreal.MaterialEditingLibrary.connect_material_expressions(custom, "", alpha_mask, "")

    # UI 머터리얼은 RGB와 Alpha를 명시적으로 분리해서 연결한다.
    unreal.MaterialEditingLibrary.connect_material_property(rgb_mask, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(alpha_mask, "", unreal.MaterialProperty.MP_OPACITY)

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)

    instance = tools.create_asset(INSTANCE_NAME, MATERIAL_PACKAGE_PATH, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    if not instance:
        raise RuntimeError("Failed to create capture ring material instance.")

    unreal.MaterialEditingLibrary.set_material_instance_parent(instance, material)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(instance, "Progress", 0.0)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(instance, "Chance", 0.0)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(instance, "IsCapturing", 0.0)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        instance,
        "ProgressColor",
        unreal.LinearColor(0.05, 1.0, 0.72, 1.0),
    )
    unreal.MaterialEditingLibrary.update_material_instance(instance)
    unreal.EditorAssetLibrary.save_loaded_asset(instance)

    return material


material = build_material()
unreal.log(f"[PWCaptureUI] Created procedural {MATERIAL_PACKAGE_PATH}/{MATERIAL_NAME}, {MATERIAL_PACKAGE_PATH}/{INSTANCE_NAME}")
