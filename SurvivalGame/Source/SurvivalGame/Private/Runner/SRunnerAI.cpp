// Copyright 2020 Tom Looman, and additional copyright holders as specified in LICENSE.md

#include "Runner/SRunnerAI.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

// Sets default values
ASRunnerAI::ASRunnerAI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASRunnerAI::BeginPlay()
{
	Super::BeginPlay();
}

void ASRunnerAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 플레이어 정보 얻기 (0번 인덱스 플레이어)
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerChar) return;

	// 2. 거리 계산
	const FVector PlayerLoc = PlayerChar->GetActorLocation();
	const FVector RunnerLoc = GetActorLocation();
	const float Distance = FVector::Dist(PlayerLoc, RunnerLoc);

	// 3. 가까워지면 반대방향 도망치기
	if (Distance < EscapeDistance)
	{
		FVector AwayFromPlayerDir = RunnerLoc - PlayerLoc;
		AwayFromPlayerDir.Z = 0; // 수평 방향(2D)만 도망
		AwayFromPlayerDir.Normalize();

		AddMovementInput(AwayFromPlayerDir, EscapeSpeed);
	}
}

void ASRunnerAI::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}