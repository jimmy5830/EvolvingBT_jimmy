
#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindLocationNearLastKnownEnemy.generated.h"

/**
* Blackboard Task - Finds a random position near the last known enemy location.
*/
UCLASS()
class SURVIVALGAME_API UBTTask_FindLocationNearLastKnownEnemy : public UBTTask_BlackboardBase
{
	GENERATED_BODY()	

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
