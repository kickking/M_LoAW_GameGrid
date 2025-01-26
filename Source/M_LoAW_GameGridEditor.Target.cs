// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class M_LoAW_GameGridEditorTarget : TargetRules
{
	public M_LoAW_GameGridEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("M_LoAW_GameGrid");
		ExtraModuleNames.Add("M_LoAW_GridData");
		ExtraModuleNames.Add("M_LoAW_Terrain");
		ExtraModuleNames.Add("M_LoAW_GameBase");
	}
}
