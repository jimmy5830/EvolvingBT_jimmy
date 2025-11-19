// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "AI/BTTask_FindLocationNearLastKnownEnemy.h"
#include "AI/SBotWaypoint.h"
#include "AI/SZombieAIController.h"

/* AI Module includes */
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
/* This contains includes all key types like UBlackboardKeyType_Vector used below. */
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "NavigationSystem.h"



EBTNodeResult::Type UBTTask_FindLocationNearLastKnownEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ASZombieAIController* myController = Cast<ASZombieAIController>(OwnerComp.GetAIOwner());
	if (myController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	FVector lastKnownEnemyLocation;
	if (myController->TryGetLastEnemyLocation(lastKnownEnemyLocation))
	{
		// Find a position that is close to the last enemy location.
		const float searchRadius = 200.0f;

		FNavLocation resultLocation;
		UNavigationSystemV1* navSystem = UNavigationSystemV1::GetNavigationSystem(myController);
		if (navSystem && navSystem->GetRandomPointInNavigableRadius(lastKnownEnemyLocation, searchRadius, resultLocation))
		{
			/* The selected key should be "PatrolLocation" in the BehaviorTree setup */
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), resultLocation.Location);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
