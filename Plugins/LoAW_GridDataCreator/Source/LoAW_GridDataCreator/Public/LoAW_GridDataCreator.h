// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FLoAW_GridDataCreatorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
private:

	void FindActors(TSubclassOf<AActor> ActorClass, TArray<AActor*>& Actors);
	void RegisterMenus();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
