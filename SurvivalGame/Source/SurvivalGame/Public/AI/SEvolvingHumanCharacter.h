
#pragma once

#include "FitnessUpdater.h"
#include "EvolvingCharacter.h"
#include "SHumanCharacter.h"

#include "SEvolvingHumanCharacter.generated.h"

class UPopulationManager;

UCLASS( ABSTRACT )
class SURVIVALGAME_API ASEvolvingHumanCharacter : public ASHumanCharacter, public IFitnessUpdater, public IEvolvingCharacter
{
    GENERATED_BODY()

    UPROPERTY()
    int32 ChromosomeID;

    UPROPERTY( EditAnywhere, Category="AI" )
    FName EvolutionPopulationTag = TEXT("StandardHumans");

    virtual void Tick( float DeltaSeconds ) override;

protected:

    virtual float
    TakeDamage( float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser ) override;

    virtual void OnDeath( float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser ) override;

    void UpdateFitness( float DeltaSeconds );

public:
    ASEvolvingHumanCharacter( const class FObjectInitializer& ObjectInitializer );

    virtual void SetCID( int32 id, UPopulationManager* populationManager ) override;
	
    virtual int32 GetCID() override { return ChromosomeID; }

    virtual const FName& GetEvolutionPopulationTag() override
    {
        return EvolutionPopulationTag;
    }
};
