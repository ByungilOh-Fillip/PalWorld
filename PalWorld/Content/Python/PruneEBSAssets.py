import unreal

def prune_ebs_assets():
    # Directories
    target_dir = "/Game/EasyBuildingSystem"
    trash_dir = "/Game/_Trash/EBS_DeleteCandidate"

    # Exact keep candidates from EBS_ASSET_PRUNE_PLAN.md
    keep_candidates = set([
        "/Game/EasyBuildingSystem/Materials/Functions/MF_Texture_Sample_RMA",
        "/Game/EasyBuildingSystem/Materials/Functions/MF_Texture_Tune_RMA_Mask",
        "/Game/EasyBuildingSystem/Materials/Instances/Dummy/MI_CanNot_Build",
        "/Game/EasyBuildingSystem/Materials/Instances/Dummy/MI_Can_Build",
        "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Metal",
        "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Stone",
        "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Stone_Roof",
        "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Wood",
        "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Wood_LOD",
        "/Game/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Wood_Roof",
        "/Game/EasyBuildingSystem/Materials/Masters/Dummy/MM_Translucent",
        "/Game/EasyBuildingSystem/Materials/Masters/Stylized/MM_Stylized_Structures_Base",
        "/Game/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Foundation",
        "/Game/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Roof",
        "/Game/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Wall",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Metal/T_Stylized_Structures_Metal_A",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Metal/T_Stylized_Structures_Metal_Mask",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Metal/T_Stylized_Structures_Metal_N",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Metal/T_Stylized_Structures_Metal_RMA",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Roof_001_A",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Roof_001_N",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Roof_001_RMA",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Stylized_Structures_Stone_A",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Stylized_Structures_Stone_N",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Stone/T_Stylized_Structures_Stone_RMA",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Wood/T_Stylized_Structures_Wood_A",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Wood/T_Stylized_Structures_Wood_LOD_A",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Wood/T_Stylized_Structures_Wood_LOD_N",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Wood/T_Stylized_Structures_Wood_N",
        "/Game/EasyBuildingSystem/Textures/Structures/Stylized/Wood/T_Stylized_Structures_Wood_RMA",
        "/Game/EasyBuildingSystem/Textures/Utility/T_Utility_A",
        "/Game/EasyBuildingSystem/Textures/Utility/T_Utility_M",
        "/Game/EasyBuildingSystem/Textures/Utility/T_Utility_N"
    ])

    editor_asset_lib = unreal.EditorAssetLibrary
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

    unreal.log("=========================================")
    unreal.log("Starting EBS Asset Pruning Script with Registry Validation...")

    # 1. Gather all assets in target directory
    all_assets = editor_asset_lib.list_assets(target_dir, recursive=True, include_folder=False)
    
    delete_candidates = []
    for asset_path in all_assets:
        # Asset path format: /Game/Folder/AssetName.AssetName
        package_name = asset_path.split('.')[0]
        if package_name not in keep_candidates:
            delete_candidates.append(package_name)

    # Dedup
    delete_candidates = list(set(delete_candidates))
    delete_candidates_set = set(delete_candidates)

    unreal.log(f"Found {len(all_assets)} total assets in {target_dir}.")
    unreal.log(f"Found {len(keep_candidates)} keep candidates.")
    unreal.log(f"Found {len(delete_candidates)} potential delete candidates.")

    # 2. Asset Registry Validation
    safe_to_move = []
    unsafe_to_move = []

    unreal.log("Validating delete candidates against external references...")

    # Include soft/hard package references for full safety
    dependency_options = unreal.AssetRegistryDependencyOptions(
        include_soft_package_references=True,
        include_hard_package_references=True,
        include_searchable_names=False,
        include_soft_management_references=False,
        include_hard_management_references=False
    )

    for pkg in delete_candidates:
        referencers = asset_registry.get_referencers(pkg, dependency_options)
        
        is_safe = True
        unsafe_refs = []
        for ref in referencers:
            ref_str = str(ref)
            # If the referencer is NOT in the delete_candidates list,
            # it is outside our isolated delete group, meaning it's still being used!
            if ref_str not in delete_candidates_set:
                is_safe = False
                unsafe_refs.append(ref_str)
        
        if is_safe:
            safe_to_move.append(pkg)
        else:
            unsafe_to_move.append((pkg, unsafe_refs))

    unreal.log(f"Validation Complete: {len(safe_to_move)} SAFE to move, {len(unsafe_to_move)} UNSAFE.")

    for unsafe_pkg, refs in unsafe_to_move:
        unreal.log_warning(f"[SKIPPING] {unsafe_pkg}")
        unreal.log_warning(f"  -> Referenced externally by: {', '.join(refs)}")

    # 3. Move Safe Assets
    if len(safe_to_move) > 0:
        if not editor_asset_lib.does_directory_exist(trash_dir):
            editor_asset_lib.make_directory(trash_dir)
            
        unreal.log(f"Moving {len(safe_to_move)} assets to {trash_dir}...")
        
        moved_count = 0
        with unreal.ScopedSlowTask(len(safe_to_move), "Moving unused assets to trash...") as slow_task:
            slow_task.make_dialog(True)
            for pkg in safe_to_move:
                if slow_task.should_cancel():
                    break
                slow_task.enter_progress_frame(1)
                
                asset_name = pkg.split('/')[-1]
                new_path = f"{trash_dir}/{asset_name}"
                
                # Handle potential name collisions
                collision_idx = 1
                while editor_asset_lib.does_asset_exist(new_path):
                    new_path = f"{trash_dir}/{asset_name}_{collision_idx}"
                    collision_idx += 1
                    
                success = editor_asset_lib.rename_asset(pkg, new_path)
                if success:
                    moved_count += 1
                else:
                    unreal.log_error(f"Failed to move: {pkg}")

        unreal.log(f"Successfully moved {moved_count} assets.")
    else:
        unreal.log("No safe assets to move.")

    # 4. Fix up Redirectors
    unreal.log("Fixing up redirectors in EasyBuildingSystem folder...")
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    redirectors_data = []
    try:
        # UE5 approach
        redirector_filter = unreal.ARFilter(
            class_paths=[unreal.TopLevelAssetPath("/Script/CoreUObject", "ObjectRedirector")],
            package_paths=[target_dir],
            recursive_paths=True
        )
        redirectors_data = asset_registry.get_assets(redirector_filter)
    except Exception:
        # UE4 fallback
        redirector_filter = unreal.ARFilter(
            class_names=["ObjectRedirector"],
            package_paths=[target_dir],
            recursive_paths=True
        )
        redirectors_data = asset_registry.get_assets(redirector_filter)

    if redirectors_data:
        redirector_objects = []
        for r_data in redirectors_data:
            obj = r_data.get_asset()
            if obj:
                redirector_objects.append(obj)
                
        if redirector_objects:
            asset_tools.fix_up_referencers(redirector_objects)
            unreal.log(f"Fixed up {len(redirector_objects)} redirectors.")
    else:
        unreal.log("No redirectors found to fix up.")

    unreal.log("=========================================")
    unreal.log("EBS Asset Prune Script Execution Finished.")

if __name__ == '__main__':
    prune_ebs_assets()
