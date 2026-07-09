# EBS Asset Prune Plan

## Current Runtime Usage

The PalWorld building implementation should not depend on EBS gameplay Blueprints, UI, demo logic, save logic, chaos assets, audio, or maps.

Runtime usage is currently limited to:

- Static meshes for foundation, wall, roof
- Stylized structure material instances
- Preview valid/invalid dummy materials
- Material parents/functions/textures required by the used material instances

## Keep Candidates

These assets are directly referenced by C++/setup scripts or are dependency candidates of the referenced materials:

```text
/Game/EasyBuildingSystem/Materials/Functions/MF_Texture_Sample_RMA
/Game/EasyBuildingSystem/Materials/Functions/MF_Texture_Tune_RMA_Mask
/Game/EasyBuildingSystem/Materials/Instances/Dummy/MI_CanNot_Build
/Game/EasyBuildingSystem/Materials/Instances/Dummy/MI_Can_Build
/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Metal
/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Stone
/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Stone_Roof
/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Wood
/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Wood_LOD
/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Wood_Roof
/Game/EasyBuildingSystem/Materials/Masters/Dummy/MM_Translucent
/Game/EasyBuildingSystem/Materials/Masters/Stylized/MM_Stylized_Structures_Base
/Game/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Foundation
/Game/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Roof
/Game/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Wall
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Metal/T_Stylized_Structures_Metal_A
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Metal/T_Stylized_Structures_Metal_Mask
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Metal/T_Stylized_Structures_Metal_N
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Metal/T_Stylized_Structures_Metal_RMA
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Roof_001_A
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Roof_001_N
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Roof_001_RMA
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Stylized_Structures_Stone_A
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Stylized_Structures_Stone_N
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Stylized_Structures_Stone_RMA
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Wood/T_Stylized_Structures_Wood_A
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Wood/T_Stylized_Structures_Wood_LOD_A
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Wood/T_Stylized_Structures_Wood_LOD_N
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Wood/T_Stylized_Structures_Wood_N
/Game/EasyBuildingSystem/Textures/Structures/Stylized/Wood/T_Stylized_Structures_Wood_RMA
/Game/EasyBuildingSystem/Textures/Utility/T_Utility_A
/Game/EasyBuildingSystem/Textures/Utility/T_Utility_M
/Game/EasyBuildingSystem/Textures/Utility/T_Utility_N
```

## Delete Candidates

First-pass delete candidates by top-level folder:

```text
Blueprints: 51 assets
Materials: 81 assets
Meshes: 116 assets
Textures: 21 assets
```

High-confidence delete candidates:

- `/Game/EasyBuildingSystem/Blueprints`
  - EBS building objects
  - EBS components
  - EBS data tables
  - EBS enums/structs/interfaces
  - EBS library/save/UI assets
- Unused mesh families:
  - Dummy
  - Polygonal
  - Stylized Metal
  - Stylized Stone
  - Stylized props/tools/extra structures
- Unused material families:
  - Polygonal materials
  - Landscape/foliage/rocks/grass materials
  - Fire/effect materials
  - Unused dummy materials
- Unused textures:
  - Polygonal structure textures
  - Door/prop textures not used by foundation/wall/roof
  - Dummy/effect textures

## Safe Deletion Procedure

1. Do not delete assets directly from Explorer.
2. In Unreal Editor, use Reference Viewer on each keep candidate and confirm all dependencies are retained.
3. Move delete candidates to a temporary folder such as `/Game/_Trash/EBS_DeleteCandidate`.
4. Run `Fix Up Redirectors`.
5. Restart editor.
6. Compile/load the building map.
7. Test:
   - B opens build UI
   - Foundation preview/spawn
   - Wall preview/spawn
   - Wood/Stone/Iron material pages
   - Preview valid/invalid materials
8. If no missing asset/material warnings appear, delete the temporary folder.

## Notes

- This is a static reference scan from current C++ and known material dependency strings. Unreal Asset Registry validation is still required before permanent deletion.
- `PW_ExtractEBSReference.py` and `EBS_ReferenceDump.json` are reference/debug artifacts, not runtime dependencies.
