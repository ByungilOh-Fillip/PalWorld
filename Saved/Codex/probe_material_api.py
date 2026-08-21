import unreal

names = [
    name for name in dir(unreal.MaterialEditingLibrary)
    if "expression" in name.lower() or "material" in name.lower() or "connect" in name.lower()
]
unreal.log("MaterialEditingLibrary APIs:")
for name in sorted(names):
    unreal.log(name)

for cls_name in [
    "MaterialExpressionCustom",
    "MaterialExpressionTextureCoordinate",
    "MaterialExpressionScalarParameter",
    "MaterialExpressionVectorParameter",
    "MaterialExpressionTextureObjectParameter",
    "MaterialExpressionComponentMask",
]:
    unreal.log(f"{cls_name}: {getattr(unreal, cls_name, None)}")
