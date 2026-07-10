# Building System Integration Plan

## Goal

Bring the useful parts of `EasyBuildingSystemV10` into `PalWorld` as a foundation for a PalWorld-specific building system.

The first target is a minimal building kit:

- Foundation
- Wall
- Roof
- Required meshes, materials, textures, data, and support blueprints

Do not import the full demo project behavior as final gameplay code. EasyBuildingSystem is used as an asset/prototype base, while PalWorld owns the final gameplay rules.

## Source Project

- Path: `C:\Users\Admin\Documents\Unreal Projects\EasyBuildingSystemV10`
- Project file: `EasyBuildingSystemv10.uproject`
- Unreal version: `5.8`
- Content root: `Content\EasyBuildingSystem`
- Has no `Source` folder. The package is primarily Blueprint/content based.

## Target Project

- Path: `C:\workspace\Github\PalWorld\PalWorld`
- Project file: `PalWorld.uproject`
- Unreal version: `5.8`
- Target content path: `Content\EasyBuildingSystem`

Keep the imported content under `/Game/EasyBuildingSystem` to avoid unnecessary redirector and broken-reference churn.

## Integration Strategy

Use EasyBuildingSystem assets for visuals and early prototyping, but write PalWorld-specific building behavior in our own system.

Reuse or import:

- Structure actors for foundation, wall, roof
- Static meshes
- Materials and material instances
- Preview/can-build/cannot-build materials
- Data tables, enums, structs, and interfaces required by the imported assets
- Existing Blueprint logic only as reference or temporary prototype behavior

Own in PalWorld code/Blueprints:

- Player input binding
- Building mode state
- Preview placement flow
- Server-side placement validation
- Inventory/resource checks
- Base/guild/ownership rules
- Save/load integration
- Damage, repair, dismantle, and multiplayer authority rules

## Priority Order

### 1. Minimal Asset Import

Import only the assets required to open and display the core building pieces.

Initial include list:

- `Blueprints\BuildingObjects\Base`
- `Blueprints\BuildingObjects\Modular`
- `Blueprints\Components`
- `Blueprints\DataTables`
- `Blueprints\Enumerations`
- `Blueprints\Interfaces`
- `Blueprints\Structures`
- `Meshes\Structures`
- `Materials\Masters`
- `Materials\Instances`
- `Textures\Structures`

Explicitly exclude:

- `Chaos`
- `Blueprints\BuildingObjects\Chaos`
- `GC_*` geometry collection assets
- Demo maps
- Demo GameMode, demo PlayerController, demo Character as default project classes
- Mannequin assets
- Foliage and environment assets unless a material dependency requires a later targeted import

Success criteria:

- Imported assets exist under `PalWorld\Content\EasyBuildingSystem`.
- Foundation, wall, and roof Blueprints can be found in the target project.
- No full demo project settings are copied over.
- No build or compile is run by Codex.

### 2. Editor Verification

The user opens the project in Unreal Editor and checks imported assets.

Success criteria:

- `BP_EBS_Building_Foundation` opens.
- `BP_EBS_Building_Wall` opens.
- `BP_EBS_Building_Roof` opens.
- Mesh and material references are either valid or listed as missing for targeted follow-up import.

### 3. Data Review

Review EBS data tables and decide the minimal PalWorld-facing data shape.

Candidate imported data:

- `DT_EBS_BuildingObjects`
- `DT_EBS_BuildingLists`
- `DT_EBS_Requirements`
- `STR_EBS_BuildingObjectSettings`
- `STR_EBS_SnapSettings`
- `STR_EBS_Requirements`

Success criteria:

- Foundation, wall, and roof can be represented by a small PalWorld building definition.
- Required class, display name, cost, preview mesh/material, and placement category are identifiable.

### 4. PalWorld Building Component Skeleton

Create a PalWorld-owned component instead of letting the EBS demo component own gameplay.

Candidate class:

- `UPalBuildingComponent`

Initial responsibilities:

- Enter/exit building mode
- Select current building definition
- Spawn/update preview actor
- Rotate preview
- Request placement
- Clear preview

Success criteria:

- The flow can be driven by function calls without input binding.
- No resource, save, or multiplayer complexity is added yet.

### 5. Minimal Placement Rules

Implement only the first useful placement pass.

Initial rules:

- Foundation traces to ground.
- Wall and roof prefer valid snap points.
- Placement checks simple overlap blocking.
- Invalid preview uses cannot-build material.

Success criteria:

- Foundation can be previewed and placed on ground.
- Wall can snap to a foundation or another wall.
- Roof can snap to a wall/top support.
- Overlap blocks placement.

### 6. Input Binding

Wire the component to player input after the behavior exists.

Candidate inputs:

- Toggle building mode
- Confirm placement
- Cancel placement
- Rotate preview
- Cycle building piece

Success criteria:

- Player can place foundation, wall, and roof manually in PIE.

### 7. Resources And Inventory

Connect placement to PalWorld's item/resource model.

Success criteria:

- Placement checks cost.
- Placement consumes resources only after successful placement.
- Failure reason can be surfaced to UI/log.

### 8. Save And Load

Persist placed buildings.

Success criteria:

- Class or definition ID, transform, owner/base metadata, and durability can be saved.
- Restarting the level restores placed structures.

### 9. Multiplayer Authority

Move final validation to the server.

Success criteria:

- Client sends placement request.
- Server validates transform, overlap, resources, ownership, and base rules.
- Server spawns replicated building actor.

## Current Task Log

- `2026-07-09`: Confirmed both projects are Unreal `5.8`.
- `2026-07-09`: Confirmed EasyBuildingSystem is Blueprint/content based with no `Source` folder.
- `2026-07-09`: Decided to import minimal foundation/wall/roof assets first and keep gameplay ownership in PalWorld code.
- `2026-07-09`: Imported the first minimal asset set into `PalWorld\Content\EasyBuildingSystem`.
- `2026-07-09`: Verified `BP_EBS_Building_Foundation`, `BP_EBS_Building_Wall`, and `BP_EBS_Building_Roof` are present in the target project.
- `2026-07-09`: Verified no `Chaos` directory or `GC_*` assets were imported in the first pass.
- `2026-07-09`: Ran UnrealEditor-Cmd reference checks. The first `ResavePackages` pass exposed missing EBS helper functions because `BP_EBS_Library` was not included.
- `2026-07-09`: Added `Blueprints\Game\BP_EBS_Library.uasset` only. Demo `BP_EBS_GameMode`, `BP_EBS_PlayerController`, `BP_EBS_SaveGame`, and `BP_EBS_SaveLibrary` were not kept.
- `2026-07-09`: Re-copied the EBS Base, Modular, and Components Blueprint folders from the source project after adding the helper library.
- `2026-07-09`: Verified the core import with a targeted Unreal Python check. The following 10 assets loaded and compiled with 0 load failures: `BP_EBS_Library`, `BP_EBS_Building_BaseObject`, `BP_EBS_Building_FloorObject`, `BP_EBS_Building_WallObject`, `BP_EBS_BuildingComponent`, `BP_EBS_InteractionComponent`, `BP_EBS_ResourcesComponent`, `BP_EBS_Building_Foundation`, `BP_EBS_Building_Wall`, and `BP_EBS_Building_Roof`.
- `2026-07-09`: Added `Textures\Utility` after a dummy structure material warning. `MM_Dummy_Structures_Base` still reports two empty `TextureSample` inputs on SM5, but the core Foundation/Wall/Roof Blueprint load and compile check succeeds.
