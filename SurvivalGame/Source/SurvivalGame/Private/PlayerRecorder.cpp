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

	//float DamagePotential = 0.f;
	//float Survivability = 0.f;
	//float Safety = 0.f;

	//if (TargetEnemy)
	//{
	//	float Dist = Data.DistanceToEnemy;

	//	float NormDist = 1.0f - FMath::Clamp(Dist / 1000.f, 0.f, 1.f);
	//	float AttackBonus = bIsAttackingFlag ? 0.5f : 0.f;

	//	DamagePotential = NormDist + AttackBonus;
	//	Survivability = FMath::Clamp(Dist / 1000.f, 0.f, 1.f);

	//	if (Dist < 200.f)       Safety = -1.0f;
	//	else if (Dist < 400.f)  Safety = -0.5f;
	//	else                    Safety = 0.2f;
	//}

	//Data.SubReward.Empty();
	//Data.SubReward.Add(DamagePotential);
	//Data.SubReward.Add(Survivability);
	//Data.SubReward.Add(Safety);

	// ====(3) Sub-Reward 계산===수정

//// 행동 목록
//	TArray<FString> Actions = { "Chase", "Patrol", "Idle", "Attack" };
//
//	// 최종 저장될 SubRewards
//	TArray<TArray<float>> SubRewardList;
//
//	float Dist = Data.DistanceToEnemy;
//	float NormDist = (TargetEnemy && Dist > 0) ? (1.0f - FMath::Clamp(Dist / 1000.f, 0.f, 1.f)) : 0.f;
//
//	bool Found = Data.bTargetFound;
//	float AttackBonus = (bIsAttackingFlag && Found) ? 1.0f : 0.f;
//
//	// 기본 Safety 계산
//	float BaseSafety = 0.f;
//	if (Dist > 0)
//	{
//		if (Dist < 200.f)      BaseSafety = -1.0f;
//		else if (Dist < 400.f) BaseSafety = -0.5f;
//		else                   BaseSafety = 0.2f;
//	}
//
//	// 각 Action별 계산
//	for (const FString& Act : Actions)
//	{
//		float DamagePotential = 0.f;
//		float Survivability = 0.f;
//		float Safety = 0.f;
//
//		if (Act == "Chase")
//		{
//			DamagePotential = NormDist;
//			if (!Found) DamagePotential *= 0.5f;   // 적을 못 봤으면 Chase 효과 반감
//
//			Survivability = 1.f - NormDist;
//			Safety = BaseSafety - 0.2f;
//		}
//		else if (Act == "Patrol")
//		{
//			DamagePotential = Found ? 0.2f : 0.1f; // 적을 발견했다면 보상↑
//			Survivability = 0.6f;
//			Safety = 0.3f;
//		}
//		else if (Act == "Idle")
//		{
//			DamagePotential = 0.05f;
//			Survivability = 0.5f;
//			Safety = Found ? -0.2f : 0.4f; // 적을 보고 가만 있음 → 위험
//		}
//		else if (Act == "Attack")
//		{
//			if (!Found)
//			{
//				DamagePotential = -1.0f; // 못 보는데 공격 → 안 좋은 행동
//			}
//			else
//			{
//				DamagePotential = NormDist + AttackBonus * 1.5f;
//			}
//
//			Survivability = -NormDist;
//			Safety = BaseSafety - 0.5f;
//		}
//
//		TArray<float> vec;
//		vec.Add(DamagePotential*5);
//		vec.Add(Survivability*5);
//		vec.Add(Safety*5);
//
//		SubRewardList.Add(vec);
//	}
//
//	Data.SubRewards = SubRewardList;


float Dist = Data.DistanceToEnemy;
float Found = Data.bTargetFound ? 1.0f : 0.0f;
float IsAtk = Data.bIsAttacking ? 1.0f : 0.0f;
float HP = Data.Health;

float Close = FMath::Clamp((600 - Dist) / 600.f, 0.f, 1.f);
float Far = FMath::Clamp(Dist / 1200.f, 0.f, 1.f);
float LowHP = FMath::Clamp((50 - HP) / 50.f, 0.f, 1.f);

TArray<FString> Actions = { "Chase", "Patrol", "Idle", "Attack" };
TArray<TArray<float>> SubRewardList;

for (auto& Act : Actions)
{
	float d = 0.f, s = 0.f, safe = 0.f;

	if (Act == "Chase")
	{
		d = Found * Close * 2;
		s = -Close;
		safe = -0.5f * Close;
	}
	else if (Act == "Patrol")
	{
		d = (1 - Found) * 0.5f;
		s = 0.5f + 0.3f * Far;
		safe = 0.5f;
	}
	else if (Act == "Idle")
	{
		d = -Found * 0.5f;
		s = LowHP * 2.0f;
		safe = 0.2f + 0.3f * Far;
	}
	else if (Act == "Attack")
	{
		d = Found * Close * 3 - (1 - Found) * 2;
		s = -Close - LowHP;
		safe = -1.5f * Close;
	}

	TArray<float> vec;
	vec.Add(d * 5);
	vec.Add(s * 5);
	vec.Add(safe * 5);
	SubRewardList.Add(vec);
}

Data.SubRewards = SubRewardList;


	//==============================================
	
	// 2. 배열에 저장
	History.Add(Data);

}

void UPlayerRecorder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SaveToJson();
	Super::EndPlay(EndPlayReason);
}

//void UPlayerRecorder::SaveToJson()
//
//
//{
//	// JSON 배열 생성
//	TArray<TSharedPtr<FJsonValue>> JsonArray;
//
//	for (const FRecordData& Data : History)
//	{
//		TSharedPtr<FJsonObject> Row = MakeShareable(new FJsonObject);
//
//		Row->SetNumberField("Time", Data.Time);
//		Row->SetNumberField("Health", Data.Health);
//
//		// 위치 정보
//		TSharedPtr<FJsonObject> Pos = MakeShareable(new FJsonObject);
//		Pos->SetNumberField("X", Data.Location.X);
//		Pos->SetNumberField("Y", Data.Location.Y);
//		Pos->SetNumberField("Z", Data.Location.Z);
//		Row->SetObjectField("Position", Pos);
//
//		// DistanceToEnemy
//		Row->SetNumberField("DistanceToEnemy", Data.DistanceToEnemy);
//
//		// IsAttacking
//		Row->SetBoolField("IsAttacking", Data.bIsAttacking);
//
//		// TargetFound
//		Row->SetBoolField("TargetFound", Data.bTargetFound);
//
//		// action 
//		Row->SetStringField("Action", Data.Action);
//
//	
//		// ---- SubRewards: 2D 배열로 쓰기 ----
//		TArray<TSharedPtr<FJsonValue>> Outer;
//
//		for (const TArray<float>& SRRow : Data.SubRewards)
//		{
//			TArray<TSharedPtr<FJsonValue>> Inner;
//			for (float v : SRRow)
//				Inner.Add(MakeShareable(new FJsonValueNumber(v)));
//
//			Outer.Add(MakeShareable(new FJsonValueArray(Inner)));
//		}
//
//		Row->SetArrayField("SubRewards", Outer);
//
//
//		JsonArray.Add(MakeShareable(new FJsonValueObject(Row)));
//	}
//
//	// 최종 객체
//	TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);
//	Root->SetArrayField("SessionData", JsonArray);
//
//	// 문자열로 변환
//	FString OutputString;
//	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
//	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
//
//	// 파일 저장 경로: 프로젝트폴더/Saved/Recordings/
//	FString FileName = FString::Printf(TEXT("Record_%s.json"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
//	FString FilePath = FPaths::ProjectSavedDir() + "Recordings/" + FileName;
//
//	FFileHelper::SaveStringToFile(OutputString, *FilePath);
//	}

void UPlayerRecorder::SaveToJson()
{
	// ---- Root JSON 객체 ----
	TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);

	// (1) 목적 함수 개수 K
	Root->SetNumberField(TEXT("K"), 3);

	// (2) objective_ids
	TArray<TSharedPtr<FJsonValue>> ObjIds;
	ObjIds.Add(MakeShareable(new FJsonValueString(TEXT("DamagePotential"))));
	ObjIds.Add(MakeShareable(new FJsonValueString(TEXT("Survivability"))));
	ObjIds.Add(MakeShareable(new FJsonValueString(TEXT("Safety"))));
	Root->SetArrayField(TEXT("objective_ids"), ObjIds);


	// (3) steps 배열
	TArray<TSharedPtr<FJsonValue>> StepsArray;

	// 행동 목록 (Tick에서 SubRewards를 계산할 때 사용한 순서와 동일해야 함)
	TArray<FString> Actions = { TEXT("Chase"), TEXT("Patrol"), TEXT("Idle"), TEXT("Attack") };

	for (const FRecordData& Data : History)
	{
		TSharedPtr<FJsonObject> StepObj = MakeShareable(new FJsonObject);

		// ---- state ----
		TSharedPtr<FJsonObject> StateObj = MakeShareable(new FJsonObject);
		StateObj->SetNumberField(TEXT("time"), Data.Time);
		StateObj->SetNumberField(TEXT("x"), Data.Location.X);
		StateObj->SetNumberField(TEXT("y"), Data.Location.Y);
		StateObj->SetNumberField(TEXT("z"), Data.Location.Z);
		StateObj->SetNumberField(TEXT("hp"), Data.Health);
		StateObj->SetNumberField(TEXT("distance_to_enemy"), Data.DistanceToEnemy);
		StateObj->SetBoolField(TEXT("is_attacking"), Data.bIsAttacking);
		StateObj->SetBoolField(TEXT("target_found"), Data.bTargetFound);

		StepObj->SetObjectField(TEXT("state"), StateObj);


		// ---- actions ----
		TArray<TSharedPtr<FJsonValue>> ActionsArr;
		for (const FString& Act : Actions)
			ActionsArr.Add(MakeShareable(new FJsonValueString(Act)));

		StepObj->SetArrayField(TEXT("actions"), ActionsArr);


		// ---- chosen_action_index ----
		int32 ChosenIdx = Actions.IndexOfByKey(Data.Action);
		if (ChosenIdx == INDEX_NONE)
			ChosenIdx = 0;

		StepObj->SetNumberField(TEXT("chosen_action_index"), ChosenIdx);


		// ---- sub_rewards (2D 배열) ----
		TArray<TSharedPtr<FJsonValue>> RewardsOuter;

		for (const TArray<float>& SRRow : Data.SubRewards)
		{
			TArray<TSharedPtr<FJsonValue>> Inner;
			for (float v : SRRow)
				Inner.Add(MakeShareable(new FJsonValueNumber(v)));

			RewardsOuter.Add(MakeShareable(new FJsonValueArray(Inner)));
		}

		StepObj->SetArrayField(TEXT("sub_rewards"), RewardsOuter);

		// Step 추가
		StepsArray.Add(MakeShareable(new FJsonValueObject(StepObj)));
	}

	Root->SetArrayField(TEXT("steps"), StepsArray);


	// ---- 문자열 변환 ----
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	// ---- 파일 저장 ----
	FString FileName = FString::Printf(TEXT("IRLRecord_%s.json"),
		*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));

	FString FilePath = FPaths::ProjectSavedDir() + TEXT("Recordings/") + FileName;

	FFileHelper::SaveStringToFile(OutputString, *FilePath);
}





