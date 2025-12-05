// Copyright 2020 Tom Looman, and additional copyright holders as specified in LICENSE.md

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dom/JsonObject.h"
#include "PlayerRecorder.generated.h"

// 한 프레임의 정보를 저장할 구조체
struct FRecordData
{
	float Time;
	FVector Location;
	FRotator Rotation;
	float Health; // 블루프린트에서 받아올 체력 값
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

	// 블루프린트에서 현재 체력값을 넣어줄 변수
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recorder")
	float CurrentHealth = 100.0f;

private:
	// 데이터를 모아둘 배열
	TArray<FRecordData> History;
	float StartTime;

	// 저장 함수
	void SaveToJson();
};