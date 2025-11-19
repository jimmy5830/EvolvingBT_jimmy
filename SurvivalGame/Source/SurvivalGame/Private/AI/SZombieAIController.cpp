// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AI/SZombieAIController.h"

#include "EvolvingBehavior.h"

#include "EvolutionControlActor.h"

#include "AI/SZombieCharacter.h"

/* AI Specific includes */
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTreeUtils.h"
#include "Navigation/CrowdFollowingComponent.h"

#include "SurvivalGameInstance.h"

ASZombieAIController::ASZombieAIController(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>( TEXT( "BehaviorComp" ) );
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>( TEXT( "BlackboardComp" ) );

    /* Match with the AI/ZombieBlackboard */
    PatrolLocationKeyName = "PatrolLocation";
    CurrentWaypointKeyName = "CurrentWaypoint";
    BotTypeKeyName = "BotType";
    TargetEnemyKeyName = "TargetEnemy";
	LastEnemyLocationKeyName = "LastKnownEnemyLocation";

    /* Initializes PlayerState so we can assign a team index to AI */
    bWantsPlayerState = true;
}

void ASZombieAIController::OnPossess( class APawn* InPawn )
{
    Super::OnPossess( InPawn );
    
	UpdateBehaviorTree();
}

UBehaviorTree* ASZombieAIController::RetrieveBehaviorTree()
{
    ASZombieCharacter* ZombieBot = Cast<ASZombieCharacter>( GetPawn() );
	if (!ZombieBot)
	{
		return nullptr;
	}

	if ( BehaviorComp->TreeHasBeenStarted() )
	{
		// For default implementation, don't stop an already-running tree to put in the default tree.
		// But subclasses can override to replace the running tree with their own.
		return nullptr;
	}

    FName newName = MakeUniqueObjectName(
        ZombieBot->BehaviorTree->GetOuter(), UBehaviorTree::StaticClass(), TEXT( "BehaviorTree_Copy" ) );
    UBehaviorTree* newTree = Cast<UBehaviorTree>(
        DuplicateObject( ZombieBot->BehaviorTree, ZombieBot->BehaviorTree->GetOuter(), newName ) );
    return newTree;
}

void ASZombieAIController::UpdateBehaviorTree()
{
    APawn* pawn = GetPawn();
    ASZombieCharacter* ZombieBot = Cast<ASZombieCharacter>( pawn );
    if( ZombieBot )
	{
	    UBehaviorTree* newTree = RetrieveBehaviorTree();
		if ( nullptr == newTree )
		{
			return;
		}

	    if( BehaviorComp->TreeHasBeenStarted() )
		{
		    BehaviorComp->StopTree( EBTStopMode::Safe );
		}

	    if( newTree->BlackboardAsset )
		{
		    BlackboardComp->InitializeBlackboard( *newTree->BlackboardAsset );

		    /* Make sure the Blackboard has the type of bot we possessed */
		    SetBlackboardBotType( ZombieBot->BotType );
		}

	    BehaviorComp->StartTree( *newTree );
    }
    
}

void ASZombieAIController::OnUnPossess()
{
    Super::OnUnPossess();

    /* Stop any behavior running as we no longer have a pawn to control */
    BehaviorComp->StopTree();
}

void ASZombieAIController::SetWaypoint( ASBotWaypoint* NewWaypoint )
{
    if( BlackboardComp )
	{
	    BlackboardComp->SetValueAsObject( CurrentWaypointKeyName, NewWaypoint );
	}
}

void ASZombieAIController::SetTargetEnemy( APawn* NewTarget )
{
    if( BlackboardComp )
	{
	    BlackboardComp->SetValueAsObject( TargetEnemyKeyName, NewTarget );
	}
}

ASBotWaypoint* ASZombieAIController::GetWaypoint()
{
    if( BlackboardComp )
	{
	    return Cast<ASBotWaypoint>( BlackboardComp->GetValueAsObject( CurrentWaypointKeyName ) );
	}

    return nullptr;
}

ASBaseCharacter* ASZombieAIController::GetTargetEnemy()
{
    if( BlackboardComp )
	{
	    return Cast<ASBaseCharacter>( BlackboardComp->GetValueAsObject( TargetEnemyKeyName ) );
	}

    return nullptr;
}

void ASZombieAIController::SetLastEnemyLocation( FVector NewLocation )
{
	if ( BlackboardComp )
	{
		BlackboardComp->SetValueAsVector( LastEnemyLocationKeyName, NewLocation );
	}
}

void ASZombieAIController::UnsetLastEnemyLocation()
{
	if (BlackboardComp)
	{
		BlackboardComp->ClearValue(LastEnemyLocationKeyName);
	}
}

bool ASZombieAIController::TryGetLastEnemyLocation(FVector& lastEnemyLocation)
{
	if ( BlackboardComp )
	{
		if ( !BlackboardComp->IsVectorValueSet(LastEnemyLocationKeyName) )
		{
			return false;
		}

		lastEnemyLocation = BlackboardComp->GetValueAsVector( LastEnemyLocationKeyName );
		return true;
	}

	return false;
}

void ASZombieAIController::SetBlackboardBotType( EBotBehaviorType NewType )
{
    if( BlackboardComp )
	{
	    BlackboardComp->SetValueAsEnum( BotTypeKeyName, (uint8)NewType );
	}
}
