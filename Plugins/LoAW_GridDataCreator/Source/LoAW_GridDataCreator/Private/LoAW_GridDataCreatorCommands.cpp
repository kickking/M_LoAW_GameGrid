// Copyright Epic Games, Inc. All Rights Reserved.

#include "LoAW_GridDataCreatorCommands.h"

#define LOCTEXT_NAMESPACE "FLoAW_GridDataCreatorModule"

void FLoAW_GridDataCreatorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "LoAW_GridDataCreator", "Execute LoAW_GridDataCreator action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
