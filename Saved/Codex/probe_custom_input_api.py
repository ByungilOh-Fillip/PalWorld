import unreal

custom = unreal.MaterialExpressionCustom()
unreal.log("MaterialExpressionCustom properties:")
for name in dir(custom):
    if "input" in name.lower() or "code" in name.lower() or "output" in name.lower():
        unreal.log(name)

for cls_name in ["CustomInput", "CustomOutput", "CustomDefine"]:
    unreal.log(f"{cls_name}: {getattr(unreal, cls_name, None)}")

ci_cls = getattr(unreal, "CustomInput", None)
if ci_cls:
    ci = ci_cls()
    unreal.log("CustomInput dir:")
    for name in dir(ci):
        if "name" in name.lower() or "input" in name.lower():
            unreal.log(name)
    try:
        ci.set_editor_property("input_name", "TestInput")
        unreal.log(f"input_name set ok: {ci.get_editor_property('input_name')}")
    except Exception as exc:
        unreal.log_error(f"input_name set failed: {exc}")

try:
    custom.set_editor_property("inputs", [ci])
    unreal.log(f"custom inputs count: {len(custom.get_editor_property('inputs'))}")
except Exception as exc:
    unreal.log_error(f"custom inputs set failed: {exc}")
