// Copyright 2020 Tom Looman, and additional copyright holders as specified in LICENSE.md

//#pragma once
//
//#include "CoreMinimal.h"
//#include "Components/ActorComponent.h"
//#include "Dom/JsonObject.h"
//#include "PlayerRecorder.generated.h"
//
//// �� �������� ������ ������ ����ü
//struct FRecordData
//{
//	float Time;
//	FVector Location;
//	FRotator Rotation;
//	float Health; // �������Ʈ���� �޾ƿ� ü�� ��
//
//	// Action 
//	FString Action;
//};
//
//UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
//class SURVIVALGAME_API UPlayerRecorder : public UActorComponent
//{
//	GENERATED_BODY()
//
//public:
//	UPlayerRecorder();
//
//protected:
//	virtual void BeginPlay() override;
//	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
//
//public:
//	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
//
//	// �������Ʈ���� ���� ü�°��� �־��� ����
//	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recorder")
//	float CurrentHealth = 100.0f;
//
//private:
//	// �����͸� ��Ƶ� �迭
//	TArray<FRecordData> History;
//	float StartTime;
//
//	// ���� �Լ�
//	void SaveToJson();
//};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dom/JsonObject.h"
#include "PlayerRecorder.generated.h"

// ========================
// UE 리플렉션 가능 구조체로 변경
// ========================
USTRUCT(BlueprintType)
struct FRecordData
{
    GENERATED_BODY()

    UPROPERTY()
    float Time;

    UPROPERTY()
    FVector Location;

    UPROPERTY()
    FRotator Rotation;

    UPROPERTY()
    float Health;

    UPROPERTY()
    bool bIsAttacking;
    
    UPROPERTY()
    float DistanceToEnemy;


    UPROPERTY()
    FString Action;
};


// ========================
// Player Recorder Component
// ========================
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
    
    UFUNCTION(BlueprintCallable, Category = "Recorder")
    void SetRecordedHealth(float NewHealth)
    {
        CurrentHealth = NewHealth;
    }
    // 블루프린트에서 Health 값을 업데이트할 수 있게 함
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recorder")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recorder")
    AActor* TargetEnemy;


    UFUNCTION(BlueprintCallable, Category = "Recorder")
    void SetIsAttacking(bool bNewState) 
    { 
        bIsAttackingFlag = bNewState;
    }
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recorder")
    bool bIsAttackingFlag = false;



    UFUNCTION(BlueprintCallable, Category = "Recorder")
    void SetRecordedAction(const FString& NewAction) {
        CurrentAction = NewAction;
    };
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recorder")
    FString CurrentAction = "Idle";



private:
    // 녹화 데이터
    UPROPERTY()
    TArray<FRecordData> History;

    float StartTime;

    void SaveToJson();
};



