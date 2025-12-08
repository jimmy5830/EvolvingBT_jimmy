// Copyright 2020 Tom Looman, and additional copyright holders as specified in LICENSE.md

#include "PlayerRecorder.h"
#include "GameFramework/Actor.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "BehaviorTree/BTNode.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


UPlayerRecorder::UPlayerRecorder()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 0.1f = 0.1�ʸ��� Tick ����
	PrimaryComponentTick.TickInterval = 0.2f;
}




void UPlayerRecorder::BeginPlay()
{
	Super::BeginPlay();
	// ���� �ð� ���
	StartTime = GetWorld()->GetTimeSeconds();
}

void UPlayerRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ����(�÷��̾�)�� ������ �ߴ�
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 1. ���� ���� ĸó
	FRecordData Data;
	Data.Time = GetWorld()->GetTimeSeconds() - StartTime;
	Data.Location = Owner->GetActorLocation();
	Data.Rotation = Owner->GetActorRotation();
	Data.Health = CurrentHealth; // �������Ʈ�� ������Ʈ���� �� ���

	// ==== (1) 적과의 거리 ====
	if (TargetEnemy)
	{
		Data.DistanceToEnemy = FVector::Dist(
			Owner->GetActorLocation(),
			TargetEnemy->GetActorLocation()
		);
	}
	else
	{
		Data.DistanceToEnemy = -1.f;  // 적이 없으면 -1
	}

	// ==== (2) 공격 중인지 여부 ====
	Data.bIsAttacking = bIsAttackingFlag;


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
	// 2. �迭�� ����
	History.Add(Data);


}

void UPlayerRecorder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SaveToJson();
	Super::EndPlay(EndPlayReason);
}

void UPlayerRecorder::SaveToJson()
{
	// JSON �迭 ����
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	for (const FRecordData& Data : History)
	{
		TSharedPtr<FJsonObject> Row = MakeShareable(new FJsonObject);

		Row->SetNumberField("Time", Data.Time);
		Row->SetNumberField("Health", Data.Health);

		// ��ġ ����
		TSharedPtr<FJsonObject> Pos = MakeShareable(new FJsonObject);
		Pos->SetNumberField("X", Data.Location.X);
		Pos->SetNumberField("Y", Data.Location.Y);
		Pos->SetNumberField("Z", Data.Location.Z);
		Row->SetObjectField("Position", Pos);

		// DistanceToEnemy
		Row->SetNumberField("DistanceToEnemy", Data.DistanceToEnemy);

		// IsAttacking
		Row->SetBoolField("IsAttacking", Data.bIsAttacking);


		// action 
		Row->SetStringField("Action", Data.Action);

	
		// SubReward 저장
		TArray<TSharedPtr<FJsonValue>> RewardArray;

		for (float v : Data.SubReward)
		{
			RewardArray.Add(MakeShareable(new FJsonValueNumber(v)));
		}

		Row->SetArrayField("SubReward", RewardArray);



		// �迭�� �߰�
		JsonArray.Add(MakeShareable(new FJsonValueObject(Row)));


	}

	// ���� ��ü
	TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);
	Root->SetArrayField("SessionData", JsonArray);

	// ���ڿ��� ��ȯ
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	// ���� ���� ���: ������Ʈ����/Saved/Recordings/
	FString FileName = FString::Printf(TEXT("Record_%s.json"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FString FilePath = FPaths::ProjectSavedDir() + "Recordings/" + FileName;

	FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

