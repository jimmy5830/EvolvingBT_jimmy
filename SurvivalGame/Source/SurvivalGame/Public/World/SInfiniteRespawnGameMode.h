
#pragma once

#include "CoreMinimal.h"
#include "World/SGameMode.h"

#include "SInfiniteRespawnGameMode.generated.h"

/**
 *
 */
UCLASS()
class SURVIVALGAME_API ASInfiniteRespawnGameMode : public ASGameMode
{
    GENERATED_BODY()

private:
	
	TArray<AController*> toRespawn;

	UPROPERTY( EditAnywhere )
	float RespawnTimeSeconds = 2.0f;

protected:

    ASInfiniteRespawnGameMode( const FObjectInitializer& ObjectInitializer );
	
	virtual void DefaultTimer();
	
public:

	virtual void Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType);

	virtual void RequestRespawn(AController* player) override;

};
