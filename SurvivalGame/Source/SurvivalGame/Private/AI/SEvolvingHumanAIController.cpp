// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AI/SEvolvingHumanAIController.h"

#include "EvolvingBehavior.h"

#include "EvolutionControlActor.h"
#include "PopulationManager.h"

#include "AI/SEvolvingHumanCharacter.h"

/* AI Specific includes */
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTreeUtils.h"

#include "SurvivalGameInstance.h"

ASEvolvingHumanAIController::ASEvolvingHumanAIController() : Super()
{
	
}

UBehaviorTree* ASEvolvingHumanAIController::RetrieveBehaviorTree()
{

    ASEvolvingHumanCharacter* HumanBot = Cast<ASEvolvingHumanCharacter>( GetPawn() );
	if (!HumanBot)
	{
		return nullptr;
	}

	if (HumanBot->GetCID() != 0)
	{
		// Ensure we don't take two population entries on the same bot unintentionally.
		return nullptr;
	}

	TArray<AEvolutionControlActor*> evolutionControlActors; 
	AEvolutionControlActor::GetEvolutionControlActorsByTag(GetWorld(), HumanBot->GetEvolutionPopulationTag(), evolutionControlActors);

    if( evolutionControlActors.Num() < 1 )
	{
	    UE_LOG( LogSurvivalGame, Warning, TEXT( "No EvolutionControlActor with tag %s found!" ), *HumanBot->GetEvolutionPopulationTag().ToString() );
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
		        this, &ASEvolvingHumanAIController::UpdateBehaviorTree);
			return nullptr;
		}
	    else if( populationManager->Register( reg ) )
		{
			populationManager->OnTrialPopulationReady().RemoveDynamic( 
				this, &ASEvolvingHumanAIController::UpdateBehaviorTree);
			APawn* pawn = GetPawn();
		    HumanBot->SetCID(reg.id, populationManager);
			return reg.bt;
		}
	}

    // Fallback to standard BT.
	return Super::RetrieveBehaviorTree();

}

void ASEvolvingHumanAIController::ReleaseRegistration()
{
	ASEvolvingHumanCharacter* HumanBot = Cast<ASEvolvingHumanCharacter>(GetPawn());
	if (!HumanBot)
	{
		return;
	}

	if (HumanBot->GetCID() == 0)
	{
		// Don't release an invalid id.
		return;
	}

	TArray<AEvolutionControlActor*> evolutionControlActors;
	AEvolutionControlActor::GetEvolutionControlActorsByTag(GetWorld(), HumanBot->GetEvolutionPopulationTag(), evolutionControlActors);

	if (evolutionControlActors.Num() < 1)
	{
		UE_LOG(LogSurvivalGame, Warning, TEXT("No EvolutionControlActor with tag %s found!"), *HumanBot->GetEvolutionPopulationTag().ToString());
		return;
	}

	// If there's more than one with this tag, just use the first.
	AEvolutionControlActor* evolutionControlActor = evolutionControlActors[0];

	UPopulationManager* populationManager = evolutionControlActor->GetPopulationManager();

	if (nullptr == populationManager)
	{
		UE_LOG(LogSurvivalGame, Error, TEXT("No population manager found."));
		return;
	}

	populationManager->ReleaseRegistration(HumanBot->GetCID());
}

void ASEvolvingHumanAIController::LogBehaviorTreeInfo( APawn* HumanPawn, UBehaviorTree* behaviorTree )
{
	// Print to the visual log.
    UBehaviorTreeUtils::PrintBehaviorTree( behaviorTree, HumanPawn );
	
	// Also print to the main log.
    UBehaviorTreeUtils::PrintBehaviorTree( behaviorTree );
}

