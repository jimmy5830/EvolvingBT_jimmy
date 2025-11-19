
#include "AI/SHumanAIController.h"

#include "EvolvingBehavior.h"

#include "EvolutionControlActor.h"

#include "AI/SHumanCharacter.h"

/* AI Specific includes */
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTreeUtils.h"
#include "Navigation/CrowdFollowingComponent.h"

#include "SurvivalGameInstance.h"

ASHumanAIController::ASHumanAIController(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>( TEXT( "BehaviorComp" ) );
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>( TEXT( "BlackboardComp" ) );

    /* Match with the AI/HumanBlackboard */
    PatrolLocationKeyName = "PatrolLocation";
    CurrentWaypointKeyName = "CurrentWaypoint";
	TargetEnemyKeyName = "TargetEnemy";

    /* Initializes PlayerState so we can assign a team index to AI */
    bWantsPlayerState = true;
}

void ASHumanAIController::OnPossess( class APawn* InPawn )
{
    Super::OnPossess( InPawn );
    
	UpdateBehaviorTree();
}

UBehaviorTree* ASHumanAIController::RetrieveBehaviorTree()
{
    ASHumanCharacter* HumanBot = Cast<ASHumanCharacter>( GetPawn() );
	if (!HumanBot)
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
        HumanBot->BehaviorTree->GetOuter(), UBehaviorTree::StaticClass(), TEXT( "BehaviorTree_Copy" ) );
    UBehaviorTree* newTree = Cast<UBehaviorTree>(
        DuplicateObject( HumanBot->BehaviorTree, HumanBot->BehaviorTree->GetOuter(), newName ) );
    return newTree;
}

void ASHumanAIController::UpdateBehaviorTree()
{
    APawn* pawn = GetPawn();
    ASHumanCharacter* HumanBot = Cast<ASHumanCharacter>( pawn );
    if( HumanBot )
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
		}

	    BehaviorComp->StartTree( *newTree );
    }
    
}

void ASHumanAIController::OnUnPossess()
{
    Super::OnUnPossess();

    /* Stop any behavior running as we no longer have a pawn to control */
    BehaviorComp->StopTree();
}

void ASHumanAIController::SetWaypoint( ASBotWaypoint* NewWaypoint )
{
    if( BlackboardComp )
	{
	    BlackboardComp->SetValueAsObject( CurrentWaypointKeyName, NewWaypoint );
	}
}

void ASHumanAIController::SetTargetEnemy(APawn* NewTarget)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(TargetEnemyKeyName, NewTarget);
	}
}

ASBotWaypoint* ASHumanAIController::GetWaypoint()
{
    if( BlackboardComp )
	{
	    return Cast<ASBotWaypoint>( BlackboardComp->GetValueAsObject( CurrentWaypointKeyName ) );
	}

    return nullptr;
}
