// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "AI/SEvolvingZombieCharacter.h"

#include "EvolutionControlActor.h"

#include "Representation/BTChromosome.h"
#include "BTChromosomeUtils.h"
#include "Fitness.h"
#include "AI/SZombieAIController.h"

// Sets default values
ASEvolvingZombieCharacter::ASEvolvingZombieCharacter( const class FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
}

void ASEvolvingZombieCharacter::SetCID( int32 id, UPopulationManager* populationManager )
{
    ChromosomeID = id;

	UFitnessTracker* fitnessTracker = populationManager->GetTracker();
    fitnessTracker->RegisterFitnessUpdater( this );

    BroadcastFitnessUpdated( GetCID(), FString( "Alive" ), 1, false );
    BroadcastFitnessUpdated( GetCID(), FString( "HasNeverMoved" ), 1, false );

    UBTChromosome* chromosome
        = populationManager->GetCurrTrial()->Get( GetCID() );
    int numNodes = BTChromosomeUtils::CountNumNodesOfType( chromosome, UBTNodeGene::StaticClass() );

    BroadcastFitnessUpdated( GetCID(), FString( "ChromosomeSize" ), numNodes );

    BroadcastFitnessUpdated( GetCID(), FString( "ChromosomeSizeUnderSix" ), std::max( 6 - numNodes, 0 ) );
}

void ASEvolvingZombieCharacter::Tick( float DeltaSeconds )
{
	bool bHadSensedTarget = false;
	if (SensedTarget())
	{
		bHadSensedTarget = true;
	}

    Super::Tick( DeltaSeconds );

	if (!SensedTarget())
	{
		if (bHadSensedTarget)
		{
			BroadcastFitnessUpdated(GetCID(), FString("LostTarget"), 1, true);
		}
	}

    UpdateFitness( DeltaSeconds );
}



void ASEvolvingZombieCharacter::OnChaseStarted(APawn* pawn, float chaseStartTime)
{
    Super::OnChaseStarted(pawn, chaseStartTime);
	BroadcastFitnessUpdated(GetCID(), FString("NumChases"), 1, true);
    if (playerChaseInfo[pawn].recentNumberChases >= recentChaseLimit)
    {
        BroadcastFitnessUpdated(GetCID(), FString("OverChaseLimit"), 1, true);
    }
}

void ASEvolvingZombieCharacter::OnChaseLost(TWeakObjectPtr<APawn> pawn, float chaseEndTime, float chaseLength)
{
    Super::OnChaseLost(pawn, chaseEndTime, chaseLength);
	BroadcastFitnessUpdated(GetCID(), FString("ChasesLost"), 1, true);
}

void ASEvolvingZombieCharacter::DoDamage(AActor* HitActor, const FPointDamageEvent& DmgEvent)
{
    Super::DoDamage(HitActor, DmgEvent);

    BroadcastFitnessUpdated( GetCID(), FString( "DamageDone" ), DmgEvent.Damage, true );
}

void ASEvolvingZombieCharacter::UpdateFitness( float DeltaSeconds )
{
    double movementDist = GetVelocity().Size2D() * DeltaSeconds;
	totalDistanceMoved += movementDist;

    if ( movementDist > 0.001 )
	{
		if ( totalDistanceMoved < MovementBonusCap )
		{
			BroadcastFitnessUpdated( GetCID(), FString( "MovementDistance" ), movementDist, true );
		}
	    BroadcastFitnessUpdated( GetCID(), FString( "HasNeverMoved" ), 0, false );
	}
	else
	{
		BroadcastFitnessUpdated(GetCID(), FString("TimeStandingStill"), DeltaSeconds, true);
	}

    ASZombieAIController* controller = Cast<ASZombieAIController>(GetController());
    if ( !controller )
    {
        return;
    }

    ASBaseCharacter* targetEnemy = controller->GetTargetEnemy();

    // Check if we have lost the target enemy, but we have a recent last known location. If so,
    // increase our fitness if we are near the target enemy's last known location, if within a time limit.
    FVector location;
    if ( !targetEnemy && controller->TryGetLastEnemyLocation( location ) &&
        (GetWorld()->TimeSeconds - LastSeenTime < NearLastEnemyLocTimeout ||
        GetWorld()->TimeSeconds - LastHeardTime < NearLastEnemyLocTimeout)  )
    {
        if ( FVector::DistSquared(location, GetActorLocation()) <= MaxLastKnownLocDistSqr )
        {
            BroadcastFitnessUpdated( GetCID(), FString("NearLastKnownEnemyLocation"), DeltaSeconds, true);
        }
    }

	if (currentChase.IsInProgress())
	{
		BroadcastFitnessUpdated(GetCID(), FString("TotalChaseTime"), DeltaSeconds, true);
	}
}

float ASEvolvingZombieCharacter::TakeDamage( float Damage,
    struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator,
    class AActor* DamageCauser )
{
    float damageTaken = Super::TakeDamage( Damage, DamageEvent, EventInstigator, DamageCauser );
    UFitness* fitness = NewObject<UFitness>( this, UFitness::StaticClass() );

    BroadcastFitnessUpdated( GetCID(), FString( "DamageTaken" ), damageTaken, true );
    return damageTaken;
}

void ASEvolvingZombieCharacter::OnDeath( float KillingDamage,
    FDamageEvent const& DamageEvent,
    APawn* PawnInstigator,
    AActor* DamageCauser )
{
    Super::OnDeath( KillingDamage, DamageEvent, PawnInstigator, DamageCauser );

    BroadcastFitnessUpdated( GetCID(), FString( "Alive" ), 0, false );
}

