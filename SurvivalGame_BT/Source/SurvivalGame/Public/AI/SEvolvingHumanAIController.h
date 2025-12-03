// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AIController.h"
#include "Player/SCharacter.h"
#include "AI/SBotWaypoint.h"
#include "SHumanAIController.h"

#include "SEvolvingHumanAIController.generated.h"

class UBehaviorTreeComponent;
class UBehaviorTree;

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASEvolvingHumanAIController : public ASHumanAIController
{
	GENERATED_BODY()

	ASEvolvingHumanAIController();

	virtual UBehaviorTree* RetrieveBehaviorTree() override;


public:

	void ReleaseRegistration();

	void LogBehaviorTreeInfo( APawn* HumanPawn, UBehaviorTree* behaviorTree );
	
	/** Returns BehaviorComp subobject **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
};
