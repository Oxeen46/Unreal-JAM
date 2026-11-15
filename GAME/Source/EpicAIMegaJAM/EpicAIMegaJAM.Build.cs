// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EpicAIMegaJAM : ModuleRules
{
	public EpicAIMegaJAM(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
            "GameplayTags",
            "NavigationSystem",
            "GameplayTasks",
            "DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"EpicAIMegaJAM",
			"EpicAIMegaJAM/Variant_Horror",
			"EpicAIMegaJAM/Variant_Horror/UI",
			"EpicAIMegaJAM/Variant_Shooter",
			"EpicAIMegaJAM/Variant_Shooter/AI",
			"EpicAIMegaJAM/Variant_Shooter/UI",
			"EpicAIMegaJAM/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
	}
}
