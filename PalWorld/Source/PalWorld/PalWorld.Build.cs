// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PalWorld : ModuleRules
{
	public PalWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraphRuntime" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"GameplayTags",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"StateTreeEditorModule",
			"GameplayStateTreeModule"
		});
		
		// 팀원 ROLE에 따라 나뉜 파트를 한번에 빌드할 때 사용
		PublicIncludePaths.AddRange(
			new string[] {
				System.IO.Path.Combine(ModuleDirectory, "LMK/public"),
				System.IO.Path.Combine(ModuleDirectory, "OBI/Components/public"),
				System.IO.Path.Combine(ModuleDirectory, "OBI/Pal/public"),
				System.IO.Path.Combine(ModuleDirectory, "OBI/Data"),
				System.IO.Path.Combine(ModuleDirectory, "OBI/AI/Evaluators/public"),
				System.IO.Path.Combine(ModuleDirectory, "OBI/AI/Tasks/public"),
				System.IO.Path.Combine(ModuleDirectory, "OBI/Skill/public"),
				System.IO.Path.Combine(ModuleDirectory, "PJH/public"),
				System.IO.Path.Combine(ModuleDirectory, "Global/Log/public"),
				System.IO.Path.Combine(ModuleDirectory, "Global/Components/public"),
				System.IO.Path.Combine(ModuleDirectory, "Global/GameplayTags/public"),
				// 추가되는 역할(ROLE) 폴더가 있다면 여기에 동일하게 경로 지정
			}
		);
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
