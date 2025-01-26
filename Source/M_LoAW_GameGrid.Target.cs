// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class M_LoAW_GameGridTarget : TargetRules
{
	public M_LoAW_GameGridTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("M_LoAW_GameGrid");
		ExtraModuleNames.Add("M_LoAW_GridData");
		ExtraModuleNames.Add("M_LoAW_Terrain");
		ExtraModuleNames.Add("M_LoAW_GameBase");
	}
}
