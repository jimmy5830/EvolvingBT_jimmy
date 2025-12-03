#include "LogfunctionLibrary.h"
#include "GameFramework/Actor.h"
// 모듈/파일 전용 로그 카테고리 정의


void ULogFunctionLibrary::LogTargetSensing(AActor* Instigator)
{
	if (Instigator)
	{
		UE_LOG(LogSurvivalGame, Log, TEXT("[SurvivalGame] Target sensing by %s"), *Instigator->GetName());
	}
	else
	{
		UE_LOG(LogSurvivalGame, Log, TEXT("[SurvivalGame] Target sensing (unknown instigator)"));
	}
}

void ULogFunctionLibrary::LogChasing(AActor* Instigator, APawn* Target)
{
	FString InstigatorName = Instigator ? Instigator->GetName() : TEXT("UnknownInstigator");
	FString TargetName = Target ? Target->GetName() : TEXT("UnknownTarget");
	UE_LOG(LogSurvivalGame, Log, TEXT("[SurvivalGame] %s is chasing %s"), *InstigatorName, *TargetName);
}

void ULogFunctionLibrary::LogDamage(ASEvolvingZombieCharacter* Zombie, float Damage, AActor* HitActor)
{
	int32 CID = Zombie ? Zombie->GetCID() : -1;
	const TCHAR* HitName = HitActor ? *HitActor->GetName() : TEXT("Unknown");
	UE_LOG(LogSurvivalGame, Log, TEXT("Zombie [%d]: DoDamage=%.2f to %s"), CID, Damage, HitName);
}
