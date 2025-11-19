
#include "AI/SEvolvingHumanCharacter.h"

#include "EvolutionControlActor.h"

#include "Representation/BTChromosome.h"
#include "BTChromosomeUtils.h"
#include "Fitness.h"
#include "AI/SEvolvingHumanAIController.h"

// Sets default values
ASEvolvingHumanCharacter::ASEvolvingHumanCharacter( const class FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
}

void ASEvolvingHumanCharacter::SetCID( int32 id, UPopulationManager* populationManager )
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

void ASEvolvingHumanCharacter::Tick( float DeltaSeconds )
{
    Super::Tick( DeltaSeconds );

    UpdateFitness( DeltaSeconds );
}

void ASEvolvingHumanCharacter::UpdateFitness( float DeltaSeconds )
{
    double movementDist = GetVelocity().Size() * DeltaSeconds;

    if( movementDist > 0.001 )
	{
	    BroadcastFitnessUpdated( GetCID(), FString( "MovementDistance" ), movementDist, true );
	    BroadcastFitnessUpdated( GetCID(), FString( "HasNeverMoved" ), 0, false );
	}
}

float ASEvolvingHumanCharacter::TakeDamage( float Damage,
    struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator,
    class AActor* DamageCauser )
{
    float damageTaken = Super::TakeDamage( Damage, DamageEvent, EventInstigator, DamageCauser );
    UFitness* fitness = NewObject<UFitness>( this, UFitness::StaticClass() );

    BroadcastFitnessUpdated( GetCID(), FString( "DamageTaken" ), damageTaken, true );
    return damageTaken;
}

void ASEvolvingHumanCharacter::OnDeath( float KillingDamage,
    FDamageEvent const& DamageEvent,
    APawn* PawnInstigator,
    AActor* DamageCauser )
{
	ASEvolvingHumanAIController* controller = Cast<ASEvolvingHumanAIController>(GetController());
	controller->ReleaseRegistration();

    Super::OnDeath( KillingDamage, DamageEvent, PawnInstigator, DamageCauser );

    BroadcastFitnessUpdated( GetCID(), FString( "Alive" ), 0, false );
}
