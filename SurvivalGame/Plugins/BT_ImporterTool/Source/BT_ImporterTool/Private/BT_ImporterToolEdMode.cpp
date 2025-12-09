// Copyright Epic Games, Inc. All Rights Reserved.

#include "BT_ImporterToolEdMode.h"
#include "BT_ImporterToolEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FBT_ImporterToolEdMode::EM_BT_ImporterToolEdModeId = TEXT("EM_BT_ImporterToolEdMode");

FBT_ImporterToolEdMode::FBT_ImporterToolEdMode()
{

}

FBT_ImporterToolEdMode::~FBT_ImporterToolEdMode()
{

}

void FBT_ImporterToolEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FBT_ImporterToolEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FBT_ImporterToolEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FBT_ImporterToolEdMode::UsesToolkits() const
{
	return true;
}




