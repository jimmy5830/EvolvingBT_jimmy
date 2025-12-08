// Copyright 2020 Tom Looman, and additional copyright holders as specified in LICENSE.md

#include "PlayerRecorder.h"
#include "GameFramework/Actor.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "BehaviorTree/BTNode.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h" 


UPlayerRecorder::UPlayerRecorder()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 0.1f = 0.1초마다 Tick 실행
	PrimaryComponentTick.TickInterval = 0.2f;
}




void UPlayerRecorder::BeginPlay()
{
	Super::BeginPlay();
	// 시작 시간 기록
	StartTime = GetWorld()->GetTimeSeconds();
}

void UPlayerRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 플레이어(PlayerRecorder 컴포넌트의 주인)가 없으면 중단
	AActor* Owner = GetOwner();
	if (!Owner) return;

	bool bCheckTargetFound = false; // TargetFound 임시 저장 변수

	// 1. 현재 상태 캡처
	FRecordData Data;
	Data.Time = GetWorld()->GetTimeSeconds() - StartTime;
	Data.Location = Owner->GetActorLocation();
	Data.Rotation = Owner->GetActorRotation();
	Data.Health = CurrentHealth; // 블루프린트가 업데이트해준 값 사용

	// ==== (1) 적과의 거리 ====
	if (TargetEnemy)
	{
		Data.DistanceToEnemy = FVector::Dist(
			Owner->GetActorLocation(),
			TargetEnemy->GetActorLocation()
		);

		// TargetEnemy를 바라보는지 확인
		FVector StartLocation;
		FRotator LookRotation;

		APawn* OwnerPawn = Cast<APawn>(Owner);
		if (OwnerPawn && OwnerPawn->GetController())
		{
			OwnerPawn->GetController()->GetPlayerViewPoint(StartLocation, LookRotation);
		}
		else
		{
			StartLocation = Owner->GetActorLocation() + FVector(0, 0, 50.f);
			LookRotation = Owner->GetActorRotation();
		}

		// 10미터(1000.0f) 앞까지 검사
		FVector EndLocation = StartLocation + (LookRotation.Vector() * 2000.0f);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);

		// 레이 발사
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartLocation,
			EndLocation,
			ECC_Visibility,
			QueryParams);

		// 맞은 물체가 타겟과 같은지 확인
		if (bHit && HitResult.GetActor() == TargetEnemy)
		{
			bCheckTargetFound = true;
		}

		// [디버그 활성화]
		//DrawDebugLine(GetWorld(), StartLocation, bHit ? HitResult.Location : EndLocation, bHit ? FColor::Green : FColor::Red, false, 0.2f);
	}
	else
	{
		Data.DistanceToEnemy = -1.f;  // 적이 없으면 -1
	}

	// ==== (2) 공격 중인지 여부 ====
	Data.bIsAttacking = bIsAttackingFlag;

	// TargetFound 레이캐스트 결과 저장
	Data.bTargetFound = bCheckTargetFound;

	// 2. GetCurrentAction() Call on Playerpawn
	// FString GetCurrentAction() (No input parameter, FString return)
	FString ActionValue;
	// 없으면 빈 문자열로 남김
	//Data.Action =  "Idle" /*8GetNodeName()*/;
	//실제 행동 이름 가져오기
	Data.Action = CurrentAction;


	// ====(3) Sub-Reward 계산===

	float DamagePotential = 0.f;
	float Survivability = 0.f;
	float Safety = 0.f;

	if (TargetEnemy)
	{
		float Dist = Data.DistanceToEnemy;

		float NormDist = 1.0f - FMath::Clamp(Dist / 1000.f, 0.f, 1.f);
		float AttackBonus = bIsAttackingFlag ? 0.5f : 0.f;

		DamagePotential = NormDist + AttackBonus;
		Survivability = FMath::Clamp(Dist / 1000.f, 0.f, 1.f);

		if (Dist < 200.f)       Safety = -1.0f;
		else if (Dist < 400.f)  Safety = -0.5f;
		else                    Safety = 0.2f;
	}

	Data.SubReward.Empty();
	Data.SubReward.Add(DamagePotential);
	Data.SubReward.Add(Survivability);
	Data.SubReward.Add(Safety);

	//==============================================
	
	// 2. 배열에 저장
	History.Add(Data);

}

void UPlayerRecorder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SaveToJson();
	Super::EndPlay(EndPlayReason);
}

void UPlayerRecorder::SaveToJson()
{
	// JSON 배열 생성
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	for (const FRecordData& Data : History)
	{
		TSharedPtr<FJsonObject> Row = MakeShareable(new FJsonObject);

		Row->SetNumberField("Time", Data.Time);
		Row->SetNumberField("Health", Data.Health);

		// 위치 정보
		TSharedPtr<FJsonObject> Pos = MakeShareable(new FJsonObject);
		Pos->SetNumberField("X", Data.Location.X);
		Pos->SetNumberField("Y", Data.Location.Y);
		Pos->SetNumberField("Z", Data.Location.Z);
		Row->SetObjectField("Position", Pos);

		// DistanceToEnemy
		Row->SetNumberField("DistanceToEnemy", Data.DistanceToEnemy);

		// IsAttacking
		Row->SetBoolField("IsAttacking", Data.bIsAttacking);

		// TargetFound
		Row->SetBoolField("TargetFound", Data.bTargetFound);

		// action 
		Row->SetStringField("Action", Data.Action);

	
		// SubReward 저장
		TArray<TSharedPtr<FJsonValue>> RewardArray;

		for (float v : Data.SubReward)
		{
			RewardArray.Add(MakeShareable(new FJsonValueNumber(v)));
		}

		Row->SetArrayField("SubReward", RewardArray);



		// 배열에 추가
		JsonArray.Add(MakeShareable(new FJsonValueObject(Row)));


	}

	// 최종 객체
	TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);
	Root->SetArrayField("SessionData", JsonArray);

	// 문자열로 변환
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	// 파일 저장 경로: 프로젝트폴더/Saved/Recordings/
	FString FileName = FString::Printf(TEXT("Record_%s.json"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FString FilePath = FPaths::ProjectSavedDir() + "Recordings/" + FileName;

	FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

