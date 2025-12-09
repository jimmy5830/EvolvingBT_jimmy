// Copyright Epic Games, Inc. All Rights Reserved.

#include "BT_ImporterTool.h"
#include "BT_ImporterToolEdMode.h"

#define LOCTEXT_NAMESPACE "FBT_ImporterToolModule"

void FBT_ImporterToolModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FBT_ImporterToolEdMode>(FBT_ImporterToolEdMode::EM_BT_ImporterToolEdModeId, LOCTEXT("BT_ImporterToolEdModeName", "BT_ImporterToolEdMode"), FSlateIcon(), true);
}

void FBT_ImporterToolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FBT_ImporterToolEdMode::EM_BT_ImporterToolEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBT_ImporterToolModule, BT_ImporterTool)