// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AIController.h"
#include "Player/SCharacter.h"
#include "AI/SBotWaypoint.h"
#include "SZombieAIController.h"

#include "SEvolvingZombieAIController.generated.h"

class UBehaviorTreeComponent;
class UBehaviorTree;

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASEvolvingZombieAIController : public ASZombieAIController
{
	GENERATED_BODY()

	ASEvolvingZombieAIController();

	virtual UBehaviorTree* RetrieveBehaviorTree() override;

public:

	void LogBehaviorTreeInfo( APawn* ZombiePawn, UBehaviorTree* behaviorTree );
	
	/** Returns BehaviorComp subobject **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
};
