// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "FitnessUpdater.h"
#include "EvolvingCharacter.h"
#include "SZombieCharacter.h"

#include "SEvolvingZombieCharacter.generated.h"

class UPopulationManager;

UCLASS( ABSTRACT )
class SURVIVALGAME_API ASEvolvingZombieCharacter : public ASZombieCharacter, public IFitnessUpdater, public IEvolvingCharacter
{
    GENERATED_BODY()

	// Constants for detecting chases.
	const float LOSE_CHASE_DISTANCE = 800.0f;
	const float LOSE_CHASE_ANGLE = 30.0f;
	const float START_CHASE_DISTANCE = 600.0f;
	const float START_CHASE_ANGLE = 20.0f;
	const float LOSE_CHASE_TIME = 1.0f;
	const float START_CHASE_TIME = 0.7f;
	const float INTERRUPTING_START_CHASE_TIME = 0.2f;
	const float INTERRUPTING_LOSING_CHASE_TIME = 0.2f;

	float totalDistanceMoved = 0.0f;

	UPROPERTY( EditAnywhere, Category="AI")
    float MovementBonusCap = 5000.0f; 

    UPROPERTY()
    int32 ChromosomeID;

    UPROPERTY( EditAnywhere, Category="AI" )
    FName EvolutionPopulationTag = TEXT("StandardZombies");

    UPROPERTY( EditAnywhere, Category="AI")
    float MaxLastKnownLocDistSqr = 40000.0f; 
    
    UPROPERTY( EditAnywhere, Category="AI")
    float NearLastEnemyLocTimeout = 8.0f; 

    UPROPERTY(EditAnywhere, Category = "AI")
    int32 recentChaseLimit = 3;

    virtual void Tick( float DeltaSeconds ) override;

protected:
    virtual void DoDamage(AActor* HitActor, const FPointDamageEvent& damage) override;

    virtual float
    TakeDamage( float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser ) override;

    virtual void OnDeath( float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser ) override;

    void UpdateFitness( float DeltaSeconds );

	virtual void OnChaseStarted(APawn* pawn, float chaseStartTime) override;

	virtual void OnChaseLost(TWeakObjectPtr<APawn> pawn, float chaseEndTime, float chaseLength) override;

public:
    ASEvolvingZombieCharacter( const class FObjectInitializer& ObjectInitializer );

    virtual void SetCID( int32 id, UPopulationManager* populationManager ) override;
	
    virtual int32 GetCID() override { return ChromosomeID; }

    virtual const FName& GetEvolutionPopulationTag() override
    {
        return EvolutionPopulationTag;
    }
};
