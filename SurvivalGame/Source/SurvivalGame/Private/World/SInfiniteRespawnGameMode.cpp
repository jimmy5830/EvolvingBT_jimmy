
#include "World/SInfiniteRespawnGameMode.h"

#include "SurvivalGameInstance.h"
#include "Player/SPlayerController.h"
#include "Player/SPlayerState.h"

ASInfiniteRespawnGameMode::ASInfiniteRespawnGameMode( const FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
}

void ASInfiniteRespawnGameMode::DefaultTimer()
{
    Super::DefaultTimer();

    if( IsMatchInProgress() )
	{
	    TArray<AController*> tempToRespawn;
	    while( toRespawn.Num() > 0 )
		{
		    ASPlayerController* playerToSpawn = Cast<ASPlayerController>( toRespawn.Pop() );
			float timeOfDeath = playerToSpawn->GetTimeOfDeath();
			float worldTime = GetWorld()->GetTimeSeconds();
		    if( PlayerCanRestart( playerToSpawn ) && GetWorld()->GetTimeSeconds() - playerToSpawn->GetTimeOfDeath() > RespawnTimeSeconds)
			{
				playerToSpawn->StopSpectating();
			    RestartPlayer( playerToSpawn );
			}
		    else
			{
			    tempToRespawn.Add( playerToSpawn );
			}
		}
	    toRespawn = tempToRespawn;
	}
}

void ASInfiniteRespawnGameMode::Killed( AController* Killer,
    AController* VictimPlayer,
    APawn* VictimPawn,
    const UDamageType* DamageType )
{
    ASPlayerState* VictimPS = VictimPlayer ? Cast<ASPlayerState>( VictimPlayer->PlayerState ) : NULL;
    if( VictimPS && !VictimPS->IsABot() )
	{
		Cast<ASPlayerController>(VictimPlayer)->SetTimeOfDeath(GetWorld()->GetTimeSeconds());
	    VictimPlayer->SetPawn( nullptr );
	    toRespawn.Add( VictimPlayer );
	}
}

void ASInfiniteRespawnGameMode::RequestRespawn(AController* player)
{
	ASPlayerState * playerState = player->GetPlayerState<ASPlayerState>();
	if (!playerState || !playerState->IsSpectator())
	{
		return;
	}
	toRespawn.Add(player);
}
