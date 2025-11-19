// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "FitnessUpdater.h"
#include "Player/SBaseCharacter.h"

#include "SZombieCharacter.generated.h"

class UPopulationManager;

USTRUCT()
struct FDetectionTimes
{
    GENERATED_BODY()
    int timesDetected;
    float lastTimeDetected;     
};

USTRUCT()
struct FChaseTimes 
{
    GENERATED_BODY()
    float lastChaseEndTime;
    float numChases;
    float recentNumberChases;
};

UCLASS( ABSTRACT )
class SURVIVALGAME_API ASZombieCharacter : public ASBaseCharacter
{
    GENERATED_BODY()

private:
    /* Last time we attacked something */
    float LastMeleeAttackTime;

    /* Time-out value to clear the sensed position of the player. Should be higher than Sense interval in the PawnSense
     * component not never miss sense ticks. */
    UPROPERTY( EditDefaultsOnly, Category = "AI" )
    float SenseTimeOut;

    /* Resets after sense time-out to avoid unnecessary clearing of target each tick */
    bool bSensedTarget;

    UPROPERTY( VisibleAnywhere, Category = "AI" )
    class UPawnSensingComponent* PawnSensingComp;


    // Constants for detecting chases.
    const float LOSE_CHASE_DISTANCE = 800.0f;
    const float LOSE_CHASE_ANGLE = 30.0f;
    const float START_CHASE_DISTANCE = 600.0f;
    const float START_CHASE_ANGLE = 20.0f;
    const float LOSE_CHASE_TIME = 1.0f;
    const float START_CHASE_TIME = 0.7f;
    const float INTERRUPTING_START_CHASE_TIME = 0.2f;
    const float INTERRUPTING_LOSING_CHASE_TIME = 0.2f;
    const float RECENT_CHASE_TIMEOUT = 30;

    void UpdateCurrentChase(float currTime);

    void MarkLosingChase(float currTime);

    void MarkStartingChase(float currTime);

    void MarkInterruptingStartOfChase(float currTime);

    void MarkInterruptingLossOfChase(float currTime);


protected:
    /*Struct to store all information involving chases with Player AIs*/
    struct ChaseInfo
    {
        TWeakObjectPtr<APawn> chasedPawn;
        float chaseStartTime = 0;
        float chaseEndTime = 0;
        float startingChaseTime = 0;
        float losingChaseTime = 0;

        bool IsInProgress()
        {
            return chaseStartTime > 0 && chaseEndTime == 0;
        }

        bool IsStarting()
        {
            return startingChaseTime > 0 && !IsInProgress();
        }

        bool IsBeingLost()
        {
            return IsInProgress() && losingChaseTime > 0;
        }
    };
    
    ChaseInfo currentChase;

    virtual void OnChaseStarted(APawn* pawn, float chaseStartTime);

    virtual void OnChaseLost(TWeakObjectPtr<APawn> pawn, float chaseEndTime, float chaseLength);

    /* Last time the player was spotted */
    float LastSeenTime;

    /* Last time the player was heard */
    float LastHeardTime;
    
    UPROPERTY()
    TMap<TWeakObjectPtr<APawn>, FDetectionTimes> playerDetectionInfo;
    TMap<TWeakObjectPtr<APawn>, FChaseTimes> playerChaseInfo;

    virtual void BeginPlay() override;

    virtual void OnPlayerDetection(APawn* pawn, float newDetectionTime);

    virtual void Tick( float DeltaSeconds ) override;

	void OnLostTarget();

    virtual bool IsSprinting() const override;

	ASBaseCharacter* GetTarget();

    /* Triggered by pawn sensing component when a pawn is spotted */
    /* When using functions as delegates they need to be marked with UFUNCTION(). We assign this function to
     * FSeePawnDelegate */
    UFUNCTION()
    void OnSeePawn( APawn* Pawn );

    UFUNCTION()
    void OnHearNoise( APawn* PawnInstigator, const FVector& Location, float Volume );

    UPROPERTY( VisibleAnywhere, Category = "Attacking" )
    UCapsuleComponent* MeleeCollisionComp;

    /* A pawn is in melee range */
    UFUNCTION()
    void OnMeleeCompBeginOverlap( class UPrimitiveComponent* OverlappedComponent,
        class AActor* OtherActor,
        class UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult );

    void OnRetriggerMeleeStrike();

    /* Deal damage to the Actor that was hit by the punch animation */
    UFUNCTION( BlueprintCallable, Category = "Attacking" )
    void PerformMeleeStrike( AActor* HitActor );

    virtual void DoDamage(AActor* HitActor, const FPointDamageEvent& damage);

    UFUNCTION( BlueprintImplementableEvent, Category="Attacking" )
    void OnDoDamage(AActor* HitActor, float damage);

    virtual float
    TakeDamage( float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser ) override;

    virtual void OnDeath( float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser ) override;

    UFUNCTION( Reliable, NetMulticast )
    void SimulateMeleeStrike();

    void SimulateMeleeStrike_Implementation();

    UPROPERTY( EditDefaultsOnly, Category = "Attacking" )
    TSubclassOf<UDamageType> PunchDamageType;

    UPROPERTY( EditDefaultsOnly, Category = "Attacking" )
    float MeleeDamage;

    UPROPERTY( EditDefaultsOnly, Category = "Attacking" )
    UAnimMontage* MeleeAnimMontage;

    /* Update the vocal loop of the zombie (idle, wandering, hunting) */
    UFUNCTION( Reliable, NetMulticast )
    void BroadcastUpdateAudioLoop( bool bNewSensedTarget );

    void BroadcastUpdateAudioLoop_Implementation( bool bNewSensedTarget );

    UAudioComponent* PlayCharacterSound( USoundCue* CueToPlay );

    UPROPERTY( EditDefaultsOnly, Category = "Sound" )
    USoundCue* SoundPlayerNoticed;

    UPROPERTY( EditDefaultsOnly, Category = "Sound" )
    USoundCue* SoundHunting;

    UPROPERTY( EditDefaultsOnly, Category = "Sound" )
    USoundCue* SoundIdle;

    UPROPERTY( EditDefaultsOnly, Category = "Sound" )
    USoundCue* SoundWandering;

    UPROPERTY( EditDefaultsOnly, Category = "Sound" )
    USoundCue* SoundAttackMelee;

    /* Timer handle to manage continous melee attacks while in range of a player */
    FTimerHandle TimerHandle_MeleeAttack;

    /* Minimum time between melee attacks */
    float MeleeStrikeCooldown;

    /* Plays the idle, wandering or hunting sound */
    UPROPERTY( VisibleAnywhere, Category = "Sound" )
    UAudioComponent* AudioLoopComp;

    virtual void PlayHit( float DamageTaken,
        struct FDamageEvent const& DamageEvent,
        APawn* PawnInstigator,
        AActor* DamageCauser,
        bool bKilled ) override;

   

    

public:
    ASZombieCharacter( const class FObjectInitializer& ObjectInitializer );

    UPROPERTY( BlueprintReadWrite, Category = "Attacking" )
    bool bIsPunching;

    /* The bot behavior we want this bot to execute, (passive/patrol) by specifying EditAnywhere we can edit this value
     * per-instance when placed on the map. */
    UPROPERTY( EditAnywhere, Category = "AI" )
    EBotBehaviorType BotType;

    /* The thinking part of the brain, steers our zombie and makes decisions based on the data we feed it from the
     * Blackboard */
    /* Assigned at the Character level (instead of Controller) so we may use different zombie behaviors while re-using
     * one controller. */
    UPROPERTY( EditDefaultsOnly, Category = "AI" )
    class UBehaviorTree* BehaviorTree;

    /* Change default bot type during gameplay */
    void SetBotType( EBotBehaviorType NewType );

	bool SensedTarget() { return bSensedTarget; }
};
