
#pragma once

#include "FitnessUpdater.h"
#include "Player/SBaseCharacter.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "SHumanCharacter.generated.h"

class UPopulationManager;
class AController;

UCLASS( ABSTRACT )
class SURVIVALGAME_API ASHumanCharacter : public ASBaseCharacter
{
    GENERATED_BODY()

    UPROPERTY( VisibleAnywhere, Category = "AI" )
    class UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(EditAnywhere, Category = "AI")
	TWeakObjectPtr<ASBaseCharacter> TargetEnemy;

	UPROPERTY(EditAnywhere, Category="Stamina")
	float MaxSprintingSeconds = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float StaminaRecoverySeconds = 16.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float ForcedRecoveryPercent = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float TotalStamina = 100;

	UPROPERTY(VisibleAnywhere, Category = "Stamina")
	float CurrStamina = 100;

	UPROPERTY(VisibleAnywhere, Category = "Stamina")
	bool IsInForcedRecovery = false;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	bool TryingToSprint = false;

    virtual void BeginPlay() override;

protected:
    virtual void Tick( float DeltaSeconds ) override;

    virtual bool IsSprinting() const override;

    /* Triggered by pawn sensing component when a pawn is spotted */
    /* When using functions as delegates they need to be marked with UFUNCTION(). We assign this function to
     * FSeePawnDelegate */
    UFUNCTION()
    void OnSeePlayer( APawn* Pawn );

	void SetNewTarget(ASBaseCharacter* NewTarget);

    UFUNCTION()
    void OnHearNoise( APawn* PawnInstigator, const FVector& Location, float Volume );

    virtual float
    TakeDamage( float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser ) override;

    virtual void OnDeath( float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser ) override;

public:
    ASHumanCharacter( const class FObjectInitializer& ObjectInitializer );

    /* The thinking part of the brain, steers our human and makes decisions based on the data we feed it from the
     * Blackboard */
    /* Assigned at the Character level (instead of Controller) so we may use different human behaviors while re-using
     * one controller. */
    UPROPERTY( EditDefaultsOnly, Category = "AI" )
    class UBehaviorTree* BehaviorTree;

	UFUNCTION(BlueprintCallable)
	ASBaseCharacter* GetTargetEnemy()
	{ 
		if (!TargetEnemy.IsValid())
		{
			return nullptr;
		}

		return TargetEnemy.Get();
	}
};
