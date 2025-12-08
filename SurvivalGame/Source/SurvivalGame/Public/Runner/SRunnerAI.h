// Copyright 2020 Tom Looman, and additional copyright holders as specified in LICENSE.md

#pragma once

#include "CoreMinimal.h"
#include "Player/SBaseCharacter.h"
#include "SRunnerAI.generated.h"

UCLASS()
class SURVIVALGAME_API ASRunnerAI : public ASBaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties (FObjectInitializer 필요)
	ASRunnerAI(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(EditAnywhere, Category="Runner AI")
	float EscapeDistance = 1000.0f;

	UPROPERTY(EditAnywhere, Category="Runner AI")
	float EscapeSpeed = 1.0f;
};