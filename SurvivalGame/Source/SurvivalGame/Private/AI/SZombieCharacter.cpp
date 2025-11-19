
// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "AI/SZombieCharacter.h"

#include "Player/SBaseCharacter.h"
#include "AI/SBotWaypoint.h"
#include "Player/SCharacter.h"
#include "Player/SPlayerState.h"
#include "AI/SZombieAIController.h"

#include "World/SGameState.h"
#include "SurvivalGameInstance.h"
#include <algorithm>

/* AI Include */
#include "Perception/PawnSensingComponent.h"

// Sets default values
ASZombieCharacter::ASZombieCharacter( const class FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
    /* Note: We assign the Controller class in the Blueprint extension of this class
            Because the zombie AIController is a blueprint in content and it's better to avoid content references in
       code.  */
    /*AIControllerClass = ASZombieAIController::StaticClass();*/

    // We are a zombie.
    SetTeam(ECharacterTeam::Zombies);

    /* Our sensing component to detect players by visibility and noise checks. */
    PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>( TEXT( "PawnSensingComp" ) );
    PawnSensingComp->bOnlySensePlayers = false;
    PawnSensingComp->SetPeripheralVisionAngle( 45.0f );
    PawnSensingComp->SightRadius = 1000;
    PawnSensingComp->HearingThreshold = 600;
    PawnSensingComp->LOSHearingThreshold = 1000;

    /* Ignore this channel or it will absorb the trace impacts instead of the skeletal mesh */
    GetCapsuleComponent()->SetCollisionResponseToChannel( COLLISION_WEAPON, ECR_Ignore );
    GetCapsuleComponent()->SetCapsuleHalfHeight( 96.0f, false );
    GetCapsuleComponent()->SetCapsuleRadius( 42.0f );

    /* These values are matched up to the CapsuleComponent above and are used to find navigation paths */
    GetMovementComponent()->NavAgentProps.AgentRadius = 42;
    GetMovementComponent()->NavAgentProps.AgentHeight = 192;

    MeleeCollisionComp = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "MeleeCollision" ) );
    MeleeCollisionComp->SetRelativeLocation( FVector( 45, 0, 25 ) );
    MeleeCollisionComp->SetCapsuleHalfHeight( 60 );
    MeleeCollisionComp->SetCapsuleRadius( 35, false );
    MeleeCollisionComp->SetCollisionResponseToAllChannels( ECR_Ignore );
    MeleeCollisionComp->SetCollisionResponseToChannel( ECC_Pawn, ECR_Overlap );
    MeleeCollisionComp->SetupAttachment( GetCapsuleComponent() );

    AudioLoopComp = CreateDefaultSubobject<UAudioComponent>( TEXT( "ZombieLoopedSoundComp" ) );
    AudioLoopComp->bAutoActivate = false;
    AudioLoopComp->bAutoDestroy = false;
    AudioLoopComp->SetupAttachment( RootComponent );

    Health = 100;
    MeleeDamage = 24.0f;
    MeleeStrikeCooldown = 1.0f;
    SprintingSpeedModifier = 3.0f;

    /* By default we will not let the AI patrol, we can override this value per-instance. */
    BotType = EBotBehaviorType::Passive;
    SenseTimeOut = 2.0f;

    /* Note: Visual Setup is done in the AI/ZombieCharacter Blueprint file */
}

void ASZombieCharacter::BeginPlay()
{
    Super::BeginPlay();

    /* This is the earliest moment we can bind our delegates to the component */
    if( PawnSensingComp )
	{
	    PawnSensingComp->OnSeePawn.AddDynamic( this, &ASZombieCharacter::OnSeePawn );
	    PawnSensingComp->OnHearNoise.AddDynamic( this, &ASZombieCharacter::OnHearNoise );
	}
    if( MeleeCollisionComp )
	{
	    MeleeCollisionComp->OnComponentBeginOverlap.AddDynamic( this, &ASZombieCharacter::OnMeleeCompBeginOverlap );
	}

    BroadcastUpdateAudioLoop( bSensedTarget );

    /* Assign a basic name to identify the bots in the HUD. */
    ASPlayerState* PS = GetPlayerState<ASPlayerState>();
    if( PS )
	{
	    PS->SetPlayerName( "ZombieBot" );
	    PS->SetIsABot(true);
	}
}

void ASZombieCharacter::Tick( float DeltaSeconds )
{
    Super::Tick( DeltaSeconds );

	if (!bSensedTarget)
	{
		return;
	}

    /* Check if the last time we sensed a player is beyond the time out value to prevent bot from endlessly following a
    * player, or if the sensed player is dead. */
    ASBaseCharacter* targetEnemy = GetTarget();
	bool bHadSensedTarget = false;
	if (SensedTarget())
	{
		bHadSensedTarget = true;
	}

	if (nullptr != targetEnemy && !targetEnemy->IsAlive())
	{
		// If we were chasing an enemy but they died, don't bother tracking their last known location.
		ASZombieAIController* AIController = Cast<ASZombieAIController>(GetController());
		if (AIController)
		{
			AIController->UnsetLastEnemyLocation();
		}
		OnLostTarget();
	}

    if ( (( GetWorld()->TimeSeconds - LastSeenTime ) > SenseTimeOut
            && ( GetWorld()->TimeSeconds - LastHeardTime ) > SenseTimeOut) )
    {
		OnLostTarget();
    }

	if (!SensedTarget())
	{
		if (bHadSensedTarget)
		{
			currentChase = ChaseInfo();
		}
	}
	else
	{
		UpdateCurrentChase(GetWorld()->GetTimeSeconds());
	}
}

void ASZombieCharacter::UpdateCurrentChase(float currTime)
{
	ASBaseCharacter* target = GetTarget();
	if (currentChase.IsInProgress() &&
		(!target || target != currentChase.chasedPawn))
	{
		// If we changed targets, end our old chase.
		currentChase.chaseEndTime = currTime;
		currentChase.startingChaseTime = 0;
		currentChase.losingChaseTime = 0;

		OnChaseLost(currentChase.chasedPawn, currentChase.chaseEndTime, currentChase.chaseEndTime - currentChase.chaseStartTime);
		
	}

	if (!target)
	{
		// If we have no target, clear all the chase info.
		currentChase.chasedPawn = nullptr;
		currentChase.chaseStartTime = 0;
		currentChase.chaseEndTime = 0;
		currentChase.startingChaseTime = 0;
		currentChase.losingChaseTime = 0;
		return;
	}

	// Track who we're potentially chasing.
	currentChase.chasedPawn = target;

	//
	// Find the relative angle to the player from our velocity vector.
	// Because cos(theta) = (a*b)/(|a||b|)
	// This should be the arccos of the dot product of the normalized vectors,
	// where the vectors are our velocity and the offset from us to the player:
	// 
	// arccos(|velocity| * |player - us|)
	//
	// And then we convert to degrees, to make it easier to reason about.
	//
	float angleToPlayer = 180.0f;
	FVector toPlayer = (target->GetActorLocation() - GetActorLocation());
	if (GetVelocity().Size() <= DELTA)
	{
		// If we're not moving at all, we're definitely not chasing after the player.
		angleToPlayer = 180.0f;
	}
	else
	{
		float dotVelocityRelativeToPlayer = FVector::DotProduct(GetVelocity().GetSafeNormal(), toPlayer.GetSafeNormal());
		angleToPlayer = FMath::RadiansToDegrees(FMath::Abs(FMath::Acos(dotVelocityRelativeToPlayer)));
	}

	if (currentChase.IsInProgress())
	{
		if (GetSquaredDistanceTo(target) > (LOSE_CHASE_DISTANCE * LOSE_CHASE_DISTANCE) ||
			angleToPlayer > LOSE_CHASE_ANGLE)
		{
			// If we're in a chase but losing it, mark that info.
			MarkLosingChase(currTime);
		}
		else if (currentChase.IsBeingLost())
		{
			// If we were losing them but are back on track, mark that we're hopefully regaining the chase.
			MarkInterruptingLossOfChase(currTime);
		}
	}
	else
	{
		if (GetSquaredDistanceTo(target) < (START_CHASE_DISTANCE * START_CHASE_DISTANCE) &&
			angleToPlayer < START_CHASE_ANGLE)
		{
			// If we are within the starting angle + distance, mark that we're starting to chase.
			MarkStartingChase(currTime);
		}
		else if (currentChase.IsStarting())
		{
			// If we were starting a chase but are no longer meeting the criteria, mark that we're starting to lose them.
			MarkInterruptingStartOfChase(currTime);
		}
	}
}

void ASZombieCharacter::MarkLosingChase(float currTime)
{
	// If we're in the process of possibly losing a chase, we're definitely no longer starting to chase them.
	// (This makes sure we reset the timer for potentially regaining the player while we were starting to lose them.)
	currentChase.startingChaseTime = 0;

	if (currentChase.IsBeingLost())
	{
		if (currTime - currentChase.losingChaseTime > LOSE_CHASE_TIME)
		{
			// We have now been losing them for too long, and we have officially lost them.
			currentChase.chaseEndTime = currTime;
			currentChase.startingChaseTime = 0;
			currentChase.losingChaseTime = 0;
			OnChaseLost(currentChase.chasedPawn, currentChase.chaseEndTime, currentChase.chaseEndTime - currentChase.chaseStartTime);
					
		}
	}
	else
	{
		// Mark when we started to potentially lose the player.
		currentChase.losingChaseTime = currTime;
	}
}

void ASZombieCharacter::MarkStartingChase(float currTime)
{
	// If we're in the process of possibly starting a chase, we're definitely no longer losing them.
	// (This makes sure we reset the timer for potentially losing the player while we were starting to chase them.)
	currentChase.losingChaseTime = 0;

	if (currentChase.IsStarting())
	{
		if (currTime - currentChase.startingChaseTime > START_CHASE_TIME)
		{
			// We've officially succeeded in chasing the player.
			// Set the chase to have started when we started trying to confirm it.
			currentChase.chaseStartTime = currentChase.startingChaseTime;
			currentChase.chaseEndTime = 0;
			currentChase.startingChaseTime = 0;
			currentChase.losingChaseTime = 0;
			if (currentChase.chasedPawn.IsValid())
			{
				OnChaseStarted(currentChase.chasedPawn.Get(), currentChase.chaseStartTime);
			}
		}
	}
	else
	{
		// Mark when we started to potentially chase the player.
		currentChase.startingChaseTime = currTime;
	}
}

void ASZombieCharacter::MarkInterruptingStartOfChase(float currTime)
{
	if (!currentChase.IsStarting())
	{
		return;
	}

	if (currentChase.losingChaseTime == 0)
	{
		// Mark when we have started to lose out on our potential chase.
		currentChase.losingChaseTime = currTime;
	}

	if (currTime - currentChase.losingChaseTime > INTERRUPTING_START_CHASE_TIME)
	{
		// Reset the starting chase setup if we have lost it.
		currentChase.chaseStartTime = 0;
		currentChase.chaseEndTime = 0;
		currentChase.startingChaseTime = 0;
		currentChase.losingChaseTime = 0;
	}
}

void ASZombieCharacter::MarkInterruptingLossOfChase(float currTime)
{
	if (!currentChase.IsBeingLost())
	{
		return;
	}

	if (currentChase.startingChaseTime == 0)
	{
		// Mark when we have started to regain the chase, possibly.
		currentChase.startingChaseTime = currTime;
	}

	if (currTime - currentChase.startingChaseTime > INTERRUPTING_LOSING_CHASE_TIME)
	{
		// Reset the losing chase setup if we have regained it.
		currentChase.startingChaseTime = 0;
		currentChase.losingChaseTime = 0;
	}
}

void ASZombieCharacter::OnChaseStarted(APawn* pawn, float chaseStartTime)
{
	if (playerChaseInfo.Contains(pawn)) 
	{
		playerChaseInfo[pawn].numChases++;
		if (GetWorld()->GetTimeSeconds() - playerChaseInfo[pawn].lastChaseEndTime <= RECENT_CHASE_TIMEOUT)
		{
			playerChaseInfo[pawn].recentNumberChases++;
		}
	}
	else
	{
		playerChaseInfo.Add(pawn, { 0, 1, 1 });
	}
}

void ASZombieCharacter::OnChaseLost(TWeakObjectPtr<APawn> pawn, float chaseEndTime, float chaseLength)
{
	if (playerChaseInfo.Contains(pawn))
	{
		playerChaseInfo[pawn].lastChaseEndTime = GetWorld()->GetTimeSeconds();
	}
	else
	{
		if (pawn.IsValid())
		{
			UE_LOG(LogSurvivalGame, Warning, TEXT("No chase was started, but system detected a lost chase for pawn : %s"), *(pawn.Get()->GetName()));
		}
		else
		{
			UE_LOG(LogSurvivalGame, Warning, TEXT("No chase was started, but system detected a lost chase for an invalid pawn"));
		}
		playerChaseInfo.Add(pawn, { GetWorld()->GetTimeSeconds() , 1, 1 });
	}
}

void ASZombieCharacter::OnLostTarget()
{
	bSensedTarget = false;
	/* Reset */
	ASZombieAIController* AIController = Cast<ASZombieAIController>(GetController());
	if (AIController)
	{
		AIController->SetTargetEnemy(nullptr);
	}

	/* Stop playing the hunting sound */
	BroadcastUpdateAudioLoop(false);
}

void ASZombieCharacter::OnSeePawn( APawn* Pawn )
{
    if( !IsAlive() )
	{
	    return;
	}

    ASZombieAIController* AIController = Cast<ASZombieAIController>( GetController() );
    ASBaseCharacter* SensedPawn = Cast<ASBaseCharacter>( Pawn );
    
    if ( !SensedPawn || SensedPawn->GetTeam() == GetTeam() )
    {
        // If they're on our team, we didn't sense a target.
        return;
    }
    
    if( !bSensedTarget )
	{
	    BroadcastUpdateAudioLoop( true );
	}
    
    /* Keep track of the time the player was last sensed in order to clear the target */
    LastSeenTime = GetWorld()->GetTimeSeconds();
    bSensedTarget = true;

    if( AIController && SensedPawn->IsAlive() )
	{
        OnPlayerDetection(Pawn, LastSeenTime);
	    AIController->SetTargetEnemy( SensedPawn );
        AIController->SetLastEnemyLocation( SensedPawn->GetActorLocation() );
	}
}


void ASZombieCharacter::OnHearNoise( APawn* PawnInstigator, const FVector& Location, float Volume )
{
    if( !IsAlive() )
	{
	    return;
	}

    ASBaseCharacter* SensedPawn = Cast<ASBaseCharacter>( PawnInstigator );
    
    if ( !SensedPawn || SensedPawn->GetTeam() == GetTeam() )
    {
        // If they're on our team, we didn't sense a target.
        return;
    }
    
    if( !bSensedTarget )
	{
	    BroadcastUpdateAudioLoop( true );
	}

    bSensedTarget = true;
    LastHeardTime = GetWorld()->GetTimeSeconds();

    ASZombieAIController* AIController = Cast<ASZombieAIController>( GetController() );
    if( AIController )
	{
        OnPlayerDetection(PawnInstigator, LastHeardTime);
	    AIController->SetTargetEnemy( PawnInstigator );
        AIController->SetLastEnemyLocation( PawnInstigator->GetActorLocation() );
	}
}

void ASZombieCharacter::PerformMeleeStrike( AActor* HitActor )
{
    if( LastMeleeAttackTime > GetWorld()->GetTimeSeconds() - MeleeStrikeCooldown )
	{
	    /* Set timer to start attacking as soon as the cooldown elapses. */
	    if( !TimerHandle_MeleeAttack.IsValid() )
		{
		    // TODO: Set Timer
		}

	    /* Attacked before cooldown expired */
	    return;
	}

    if( !HitActor || HitActor == this || !IsAlive() )
	{
        return;
    }

    ASBaseCharacter* OtherPawn = Cast<ASBaseCharacter>( HitActor );
    if ( !OtherPawn || OtherPawn->GetTeam() == GetTeam() )
    {
        // If they're on our team, we didn't sense a target.
        return;
    }
    
    /* Set to prevent a zombie to attack multiple times in a very short time */
    LastMeleeAttackTime = GetWorld()->GetTimeSeconds();

    FPointDamageEvent DmgEvent;
    DmgEvent.DamageTypeClass = PunchDamageType;
    DmgEvent.Damage = MeleeDamage;

    DoDamage(HitActor, DmgEvent);

    SimulateMeleeStrike();
}

void ASZombieCharacter::DoDamage(AActor* HitActor, const FPointDamageEvent& DmgEvent)
{
    HitActor->TakeDamage( DmgEvent.Damage, DmgEvent, GetController(), this );

    OnDoDamage(HitActor, DmgEvent.Damage);
}

float ASZombieCharacter::TakeDamage( float Damage,
    struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator,
    class AActor* DamageCauser )
{
    float damageTaken = Super::TakeDamage( Damage, DamageEvent, EventInstigator, DamageCauser );
    return damageTaken;
}

void ASZombieCharacter::OnDeath( float KillingDamage,
    FDamageEvent const& DamageEvent,
    APawn* PawnInstigator,
    AActor* DamageCauser )
{
    Super::OnDeath( KillingDamage, DamageEvent, PawnInstigator, DamageCauser );
}

void ASZombieCharacter::SetBotType( EBotBehaviorType NewType )
{
    BotType = NewType;

    ASZombieAIController* AIController = Cast<ASZombieAIController>( GetController() );
    if( AIController )
	{
	    AIController->SetBlackboardBotType( NewType );
	}

    BroadcastUpdateAudioLoop( bSensedTarget );
}

UAudioComponent* ASZombieCharacter::PlayCharacterSound( USoundCue* CueToPlay )
{
    if( CueToPlay )
	{
	    return UGameplayStatics::SpawnSoundAttached(
	        CueToPlay, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true );
	}

    return nullptr;
}

void ASZombieCharacter::PlayHit( float DamageTaken,
    struct FDamageEvent const& DamageEvent,
    APawn* PawnInstigator,
    AActor* DamageCauser,
    bool bKilled )
{
    Super::PlayHit( DamageTaken, DamageEvent, PawnInstigator, DamageCauser, bKilled );

    /* Stop playing the hunting sound */
    if( AudioLoopComp && bKilled )
	{
	    AudioLoopComp->Stop();
	}
}

void ASZombieCharacter::SimulateMeleeStrike_Implementation()
{
    PlayAnimMontage( MeleeAnimMontage );
    PlayCharacterSound( SoundAttackMelee );
}

void ASZombieCharacter::OnMeleeCompBeginOverlap( class UPrimitiveComponent* OverlappedComponent,
    class AActor* OtherActor,
    class UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult )
{
    /* Stop any running attack timers */
    TimerHandle_MeleeAttack.Invalidate();

    PerformMeleeStrike( OtherActor );

    /* Set re-trigger timer to re-check overlapping pawns at melee attack rate interval */
    GetWorldTimerManager().SetTimer(
        TimerHandle_MeleeAttack, this, &ASZombieCharacter::OnRetriggerMeleeStrike, MeleeStrikeCooldown, true );
}

void ASZombieCharacter::OnRetriggerMeleeStrike()
{
    /* Apply damage to a single random pawn in range. */
    TArray<AActor*> Overlaps;
    MeleeCollisionComp->GetOverlappingActors( Overlaps, ASBaseCharacter::StaticClass() );
    for( int32 i = 0; i < Overlaps.Num(); i++ )
	{
	    ASBaseCharacter* OverlappingPawn = Cast<ASBaseCharacter>( Overlaps[i] );
	    if( OverlappingPawn )
		{
		    PerformMeleeStrike( OverlappingPawn );
		    // break; /* Uncomment to only attack one pawn maximum */
		}
	}

    /* No pawns in range, cancel the retrigger timer */
    if( Overlaps.Num() == 0 )
	{
	    TimerHandle_MeleeAttack.Invalidate();
	}
}

bool ASZombieCharacter::IsSprinting() const
{
    /* Allow a zombie to sprint when he has seen a player */
    return bSensedTarget && !GetVelocity().IsZero();
}

ASBaseCharacter * ASZombieCharacter::GetTarget()
{
	if (!bSensedTarget)
	{
		return nullptr;
	}
	ASZombieAIController* AIController = Cast<ASZombieAIController>( GetController() );
	if (!AIController)
	{
		return nullptr;
	}

	return AIController->GetTargetEnemy();
}

void ASZombieCharacter::BroadcastUpdateAudioLoop_Implementation( bool bNewSensedTarget )
{
    /* Start playing the hunting sound and the "noticed player" sound if the state is about to change */
    if( bNewSensedTarget && !bSensedTarget )
	{
	    PlayCharacterSound( SoundPlayerNoticed );

	    AudioLoopComp->SetSound( SoundHunting );
	    AudioLoopComp->Play();
	}
    else
	{
	    if( BotType == EBotBehaviorType::Patrolling )
		{
		    AudioLoopComp->SetSound( SoundWandering );
		    AudioLoopComp->Play();
		}
	    else
		{
		    AudioLoopComp->SetSound( SoundIdle );
		    AudioLoopComp->Play();
		}
	}
}

void ASZombieCharacter::OnPlayerDetection(APawn* Pawn, float newDetectionTime) 
{
    if (playerDetectionInfo.Contains(Pawn)) 
    {
        playerDetectionInfo[Pawn].timesDetected++;
        playerDetectionInfo[Pawn].lastTimeDetected = newDetectionTime;
    }
    else 
    {
        playerDetectionInfo.Add(Pawn, { 1, newDetectionTime });
    }
}