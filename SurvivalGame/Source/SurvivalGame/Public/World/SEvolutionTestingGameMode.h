// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/SInfiniteRespawnGameMode.h"

#include "SEvolutionTestingGameMode.generated.h"

/**
 *
 */
UCLASS()
class SURVIVALGAME_API ASEvolutionTestingGameMode : public ASInfiniteRespawnGameMode
{
    GENERATED_BODY()

protected:
    ASEvolutionTestingGameMode( const FObjectInitializer& ObjectInitializer );
	
	virtual bool CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget) override;
	
	virtual void DefaultTimer();
	
    virtual void SpawnBotHandler_Implementation() override;

	virtual FString InitNewPlayer(class APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
};
