
#include "World/SEvolutionTestingGameMode.h"

#include "SurvivalGameInstance.h"
#include "PopulationManager.h"
#include "Player/SPlayerController.h"
#include "Player/SPlayerState.h"
#include "EvolutionControlActor.h"
#include "AI/EvolvingCharacter.h"
#include "AI/SEvolvingZombieCharacter.h"


ASEvolutionTestingGameMode::ASEvolutionTestingGameMode( const FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
    SetBotSpawnInterval( 0.05f );
}

void ASEvolutionTestingGameMode::SpawnBotHandler_Implementation()
{
	TArray<TSubclassOf<APawn>> pawnTypesAvailable;

	for( TSubclassOf<APawn> pawnType : BotPawnTypes )
	{
		IEvolvingCharacter* evolvingChar = Cast<IEvolvingCharacter>(pawnType->GetDefaultObject());
		if ( !evolvingChar )
		{
			UE_LOG(LogSurvivalGame, Warning, TEXT("Character type in GameMode's bot types list is not a subclass of EvolvingZombieCharacter."));
			continue;
		}
		FName evolutionTag = evolvingChar->GetEvolutionPopulationTag();

		TArray<AEvolutionControlActor*> evolutionControlActors;
		AEvolutionControlActor::GetEvolutionControlActorsByTag(GetWorld(), evolutionTag, evolutionControlActors);

		if (evolutionControlActors.Num() < 1)
		{
			UE_LOG(LogSurvivalGame, Warning, TEXT("No evolution control actor with tag %s"), *evolutionTag.ToString());
			continue;
		}
		// We only expect to have one evolution control actor per population tag, so just take the first.
		AEvolutionControlActor* evolutionControlActor = evolutionControlActors[0];

		UPopulationManager* populationManager = evolutionControlActor->GetPopulationManager();

		if ( populationManager->GetPopulationRemaining() > 0)
		{
			pawnTypesAvailable.Add(pawnType);
		}
	}

	if (pawnTypesAvailable.Num() < 1 )
	{
		return;
	}

	SpawnNewBot(pawnTypesAvailable[FMath::RandRange(0, pawnTypesAvailable.Num() - 1)]);
}

bool ASEvolutionTestingGameMode::CanSpectate_Implementation( APlayerController* Viewer, APlayerState* ViewTarget )
{
    return true;
}

void ASEvolutionTestingGameMode::DefaultTimer()
{
    Super::DefaultTimer();

    if( IsMatchInProgress() )
	{
		// All zombies should always be awake
		WakeAllBots();
	}
}

FString ASEvolutionTestingGameMode::InitNewPlayer(class APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	ASPlayerController* playerController = Cast<ASPlayerController>(NewPlayerController);
	if (playerController)
	{
		playerController->StartSpectating();
		playerController->PlayerState->SetIsOnlyASpectator(true);
	}

	FString Result = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	return Result;
}