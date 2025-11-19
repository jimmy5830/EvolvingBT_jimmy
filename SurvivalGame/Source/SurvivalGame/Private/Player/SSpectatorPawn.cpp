// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "Player/SSpectatorPawn.h"
#include "World/SGameMode.h"



ASSpectatorPawn::ASSpectatorPawn(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAddDefaultMovementBindings = true;
}

void ASSpectatorPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASSpectatorPawn::StopSpectatorOnlyMode);
}

void ASSpectatorPawn::StopSpectatorOnlyMode()
{
	AController* controller = GetController();
	if (!controller)
	{
		UE_LOG(LogSurvivalGame, Warning, TEXT("Spectator pawn has no controller."));
		return;
	}

	APlayerState* playerState = controller->GetPlayerState<APlayerState>();
	
	if (!playerState)
	{
		UE_LOG(LogSurvivalGame, Warning, TEXT("Spectator pawn has no player state."));
		return;
	}

	playerState->SetIsOnlyASpectator(false);

	ASGameMode* gameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	gameMode->RequestRespawn(controller);
}