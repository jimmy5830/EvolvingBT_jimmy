
#pragma once

#include "AIController.h"
#include "Player/SCharacter.h"
#include "AI/SBotWaypoint.h"

#include "SHumanAIController.generated.h"

class UBehaviorTreeComponent;
class UBehaviorTree;

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASHumanAIController : public AAIController
{
	GENERATED_BODY()

protected:
	UBehaviorTreeComponent* BehaviorComp;

	UBlackboardComponent* BlackboardComp;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName PatrolLocationKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName CurrentWaypointKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetEnemyKeyName;

	ASHumanAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/* Called whenever the controller possesses a character bot */
	virtual void OnPossess(class APawn* InPawn) override;

	virtual void OnUnPossess() override;
	
	void UpdateBehaviorTree();
	
	virtual UBehaviorTree* RetrieveBehaviorTree();

public:

	ASBotWaypoint* GetWaypoint();

	void SetWaypoint(ASBotWaypoint* NewWaypoint);
	
	void SetTargetEnemy(APawn* NewTarget);

	/** Returns BehaviorComp subobject **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
};
