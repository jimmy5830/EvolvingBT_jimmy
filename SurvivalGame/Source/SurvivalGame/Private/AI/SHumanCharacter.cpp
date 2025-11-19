
#include "AI/SHumanCharacter.h"

#include "Player/SBaseCharacter.h"
#include "AI/SBotWaypoint.h"
#include "Player/SCharacter.h"
#include "Player/SPlayerState.h"

#include "World/SGameState.h"
#include "World/SGameMode.h"
#include "SurvivalGameInstance.h"
#include "AI/SHumanAIController.h"
#include <algorithm>

/* AI Include */
#include "Perception/PawnSensingComponent.h"

// Sets default values
ASHumanCharacter::ASHumanCharacter( const class FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
    /* Our sensing component to detect players by visibility and noise checks. */
    PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>( TEXT( "PawnSensingComp" ) );
	PawnSensingComp->bOnlySensePlayers = false;
    PawnSensingComp->SetPeripheralVisionAngle( 180.0f );
    PawnSensingComp->SightRadius = 900;
    PawnSensingComp->HearingThreshold = 600;
    PawnSensingComp->LOSHearingThreshold = 900;

    GetCapsuleComponent()->SetCapsuleHalfHeight( 96.0f, false );
    GetCapsuleComponent()->SetCapsuleRadius( 42.0f );

    /* These values are matched up to the CapsuleComponent above and are used to find navigation paths */
    GetMovementComponent()->NavAgentProps.AgentRadius = 42;
    GetMovementComponent()->NavAgentProps.AgentHeight = 192;

    Health = 100;

    SprintingSpeedModifier = 2.5f;

}

void ASHumanCharacter::BeginPlay()
{
    Super::BeginPlay();

    /* This is the earliest moment we can bind our delegates to the component */
    if( PawnSensingComp )
	{
	    PawnSensingComp->OnSeePawn.AddDynamic( this, &ASHumanCharacter::OnSeePlayer );
	    PawnSensingComp->OnHearNoise.AddDynamic( this, &ASHumanCharacter::OnHearNoise );
	}
    
    /* Assign a basic name to identify the bots in the HUD. */
    ASPlayerState* PS = GetPlayerState<ASPlayerState>();
    if( PS )
	{
	    PS->SetPlayerName( "HumanBot" );
	    PS->SetIsABot(true);
	}
}

void ASHumanCharacter::Tick( float DeltaSeconds )
{
    Super::Tick( DeltaSeconds );
	
	if (!IsAlive())
	{
		return;
	}

	if (TargetEnemy.IsValid())
	{
		if (this->GetDistanceTo(TargetEnemy.Get()) > PawnSensingComp->SightRadius)
		{
			SetNewTarget(nullptr);
		}
		else
		{
			TryingToSprint = true;
		}
	}
	else
	{
		TryingToSprint = false;
	}

	if (IsSprinting())
	{
		CurrStamina -= (TotalStamina / MaxSprintingSeconds) * DeltaSeconds;
	}

	if (CurrStamina <= 0.0f)
	{
		IsInForcedRecovery = true;
		CurrStamina = 0.0f;
	}

	if (!IsSprinting() && CurrStamina < TotalStamina)
	{
		CurrStamina += (TotalStamina / StaminaRecoverySeconds) * DeltaSeconds;
		if (CurrStamina > 100.0f)
		{
			CurrStamina = 100.0f;
		}
	
		if (IsInForcedRecovery && CurrStamina > TotalStamina * ForcedRecoveryPercent)
		{
			IsInForcedRecovery = false;
		}
	}
}

void ASHumanCharacter::OnSeePlayer( APawn* Pawn )
{
    if( !IsAlive() )
	{
	    return;
	}

	ASBaseCharacter* SensedPawn = Cast<ASBaseCharacter>(Pawn);

	if (!SensedPawn || SensedPawn->GetTeam() == GetTeam())
	{
		// If they're on our team, we didn't sense a target.
		return;
	}

	if (TargetEnemy.IsValid())
	{
		if (TargetEnemy.Get() == SensedPawn)
		{
			// We just saw the same target again.
			return;
		}

		if (this->GetDistanceTo(SensedPawn) > this->GetDistanceTo(TargetEnemy.Get()))
		{
			return;
		}
	}
	
	SetNewTarget(SensedPawn);
}

void ASHumanCharacter::SetNewTarget(ASBaseCharacter* NewTarget)
{
	TargetEnemy = NewTarget;

	ASHumanAIController* AIController = Cast<ASHumanAIController>(GetController());
	if (!AIController)
	{
		return;
	}

	AIController->SetTargetEnemy(NewTarget);
}

void ASHumanCharacter::OnHearNoise( APawn* PawnInstigator, const FVector& Location, float Volume )
{
    if( !IsAlive() )
	{
	    return;
	}
}

float ASHumanCharacter::TakeDamage( float Damage,
    struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator,
    class AActor* DamageCauser )
{
    float damageTaken = Super::TakeDamage( Damage, DamageEvent, EventInstigator, DamageCauser );
    return damageTaken;
}

void ASHumanCharacter::OnDeath( float KillingDamage,
    FDamageEvent const& DamageEvent,
    APawn* PawnInstigator,
    AActor* DamageCauser )
{
    Super::OnDeath( KillingDamage, DamageEvent, PawnInstigator, DamageCauser );
}

bool ASHumanCharacter::IsSprinting() const
{
    return TryingToSprint && !IsInForcedRecovery && CurrStamina > 0;
}