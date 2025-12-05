// Copyright 2020 Tom Looman, and additional copyright holders as specified in LICENSE.md

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dom/JsonObject.h"
#include "PlayerRecorder.generated.h"

// �� �������� ������ ������ ����ü
struct FRecordData
{
	float Time;
	FVector Location;
	FRotator Rotation;
	float Health; // �������Ʈ���� �޾ƿ� ü�� ��

	// Action 
	FString Action;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UPlayerRecorder : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerRecorder();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// �������Ʈ���� ���� ü�°��� �־��� ����
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recorder")
	float CurrentHealth = 100.0f;

private:
	// �����͸� ��Ƶ� �迭
	TArray<FRecordData> History;
	float StartTime;

	// ���� �Լ�
	void SaveToJson();
};