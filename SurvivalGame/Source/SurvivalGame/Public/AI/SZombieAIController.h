// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AIController.h"
#include "Player/SCharacter.h"
#include "AI/SBotWaypoint.h"

#include "SZombieAIController.generated.h"

class UBehaviorTreeComponent;
class UBehaviorTree;

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASZombieAIController : public AAIController
{
	GENERATED_BODY()

protected:
	UBehaviorTreeComponent* BehaviorComp;

	UBlackboardComponent* BlackboardComp;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetEnemyKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName PatrolLocationKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName CurrentWaypointKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName BotTypeKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName LastEnemyLocationKeyName;
	
	ASZombieAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/* Called whenever the controller possesses a character bot */
	virtual void OnPossess(class APawn* InPawn) override;

	virtual void OnUnPossess() override;
	
	void UpdateBehaviorTree();
	
	virtual UBehaviorTree* RetrieveBehaviorTree();

public:

	ASBotWaypoint* GetWaypoint();

	ASBaseCharacter* GetTargetEnemy();

	bool TryGetLastEnemyLocation(FVector& lastEnemyLocation);

	void SetWaypoint(ASBotWaypoint* NewWaypoint);

	void SetTargetEnemy(APawn* NewTarget); 

	void SetLastEnemyLocation(FVector NewLocation);

	void UnsetLastEnemyLocation();

	void SetBlackboardBotType(EBotBehaviorType NewType);
	
	/** Returns BehaviorComp subobject **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
};
