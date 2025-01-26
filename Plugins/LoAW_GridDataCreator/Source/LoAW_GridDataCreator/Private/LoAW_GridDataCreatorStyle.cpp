// Copyright Epic Games, Inc. All Rights Reserved.

#include "LoAW_GridDataCreatorStyle.h"
#include "LoAW_GridDataCreator.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FLoAW_GridDataCreatorStyle::StyleInstance = nullptr;

void FLoAW_GridDataCreatorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FLoAW_GridDataCreatorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FLoAW_GridDataCreatorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("LoAW_GridDataCreatorStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FLoAW_GridDataCreatorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("LoAW_GridDataCreatorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("LoAW_GridDataCreator")->GetBaseDir() / TEXT("Resources"));

	Style->Set("LoAW_GridDataCreator.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FLoAW_GridDataCreatorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FLoAW_GridDataCreatorStyle::Get()
{
	return *StyleInstance;
}
