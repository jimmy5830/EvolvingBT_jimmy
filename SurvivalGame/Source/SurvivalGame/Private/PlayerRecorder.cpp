// Copyright 2020 Tom Looman, and additional copyright holders as specified in LICENSE.md

#include "PlayerRecorder.h"
#include "GameFramework/Actor.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "BehaviorTree/BTNode.h"

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

	// 2. GetCurrentAction() Call on Playerpawn
	// FString GetCurrentAction() (No input parameter, FString return)
	FString ActionValue;
	// 없으면 빈 문자열로 남김
	Data.Action =  "Chase" /*8GetNodeName()*/;

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

		// action 
		Row->SetStringField("Action", Data.Action);

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

