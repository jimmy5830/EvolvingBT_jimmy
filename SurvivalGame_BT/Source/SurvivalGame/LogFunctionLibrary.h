#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AI/SEvolvingZombieCharacter.h" // ASEvolvingZombieCharacter에서 GetCID()를 사용하기 위해 포함
#include "LogFunctionLibrary.generated.h"

UCLASS()
class SURVIVALGAME_API ULogFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 간단한 대상 감지 로그 (원하면 파라미터 추가 가능)
	UFUNCTION(BlueprintCallable, Category = "SurvivalGame|Logging")
	static void LogTargetSensing(AActor* Instigator = nullptr);

	// 추적 로그 (원하면 대상 Pawn 추가 가능)
	UFUNCTION(BlueprintCallable, Category = "SurvivalGame|Logging")
	static void LogChasing(AActor* Instigator = nullptr, APawn* Target = nullptr);

	// 데미지 로그: ASEvolvingZombieCharacter 포인터, 데미지 값, 히트된 액터를 전달
	UFUNCTION(BlueprintCallable, Category = "SurvivalGame|Logging")
	static void LogDamage(ASEvolvingZombieCharacter* Zombie, float Damage, AActor* HitActor);
};