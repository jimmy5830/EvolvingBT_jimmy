// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AI/SEvolvingZombieAIController.h"

#include "EvolvingBehavior.h"

#include "EvolutionControlActor.h"

#include "AI/SEvolvingZombieCharacter.h"

/* AI Specific includes */
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTreeUtils.h"

#include "SurvivalGameInstance.h"

ASEvolvingZombieAIController::ASEvolvingZombieAIController() : Super()
{
	
}

UBehaviorTree* ASEvolvingZombieAIController::RetrieveBehaviorTree()
{

    ASEvolvingZombieCharacter* ZombieBot = Cast<ASEvolvingZombieCharacter>( GetPawn() );
	if (!ZombieBot)
	{
		return nullptr;
	}

	if (ZombieBot->GetCID() != 0)
	{
		// Ensure we don't take two population entries on the same bot unintentionally.
		return nullptr;
	}

	TArray<AEvolutionControlActor*> evolutionControlActors; 
	AEvolutionControlActor::GetEvolutionControlActorsByTag(GetWorld(), ZombieBot->GetEvolutionPopulationTag(), evolutionControlActors);

    if( evolutionControlActors.Num() < 1 )
	{
	    UE_LOG( LogSurvivalGame, Warning, TEXT( "No EvolutionControlActor with tag %s found!" ), *ZombieBot->GetEvolutionPopulationTag().ToString() );
	    return nullptr;
	}

	// If there's more than one with this tag, just use the first.
	AEvolutionControlActor* evolutionControlActor = evolutionControlActors[0];

	UPopulationManager* populationManager = evolutionControlActor->GetPopulationManager(); 

	if (nullptr == populationManager)
	{
		UE_LOG(LogSurvivalGame, Error, TEXT("No population manager found."));
	}
	else
	{
	    FPopulationReg reg;

	    if( !populationManager->IsTrialReady() )
		{
		    populationManager->OnTrialPopulationReady().AddDynamic(
		        this, &ASEvolvingZombieAIController::UpdateBehaviorTree);
			return nullptr;
		}
	    else if( populationManager->Register( reg ) )
		{
			populationManager->OnTrialPopulationReady().RemoveDynamic( 
				this, &ASEvolvingZombieAIController::UpdateBehaviorTree);
			APawn* pawn = GetPawn();
		    ZombieBot->SetCID(reg.id, populationManager);
			return reg.bt;
		}
	}

    // Fallback to standard BT.
	return Super::RetrieveBehaviorTree();

}

void ASEvolvingZombieAIController::LogBehaviorTreeInfo( APawn* ZombiePawn, UBehaviorTree* behaviorTree )
{
	// Print to the visual log.
    UBehaviorTreeUtils::PrintBehaviorTree( behaviorTree, ZombiePawn );
	
	// Also print to the main log.
    UBehaviorTreeUtils::PrintBehaviorTree( behaviorTree );
}

