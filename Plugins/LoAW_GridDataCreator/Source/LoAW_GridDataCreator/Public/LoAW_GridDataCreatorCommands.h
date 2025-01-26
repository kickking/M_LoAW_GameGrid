// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "LoAW_GridDataCreatorStyle.h"

class FLoAW_GridDataCreatorCommands : public TCommands<FLoAW_GridDataCreatorCommands>
{
public:

	FLoAW_GridDataCreatorCommands()
		: TCommands<FLoAW_GridDataCreatorCommands>(TEXT("LoAW_GridDataCreator"), NSLOCTEXT("Contexts", "LoAW_GridDataCreator", "LoAW_GridDataCreator Plugin"), NAME_None, FLoAW_GridDataCreatorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
