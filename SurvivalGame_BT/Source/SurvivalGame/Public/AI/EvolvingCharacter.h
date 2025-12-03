
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PopulationManager.h"
#include "EvolvingCharacter.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEvolvingCharacter : public UInterface
{
	GENERATED_BODY()
};

/**
 * @file EvolvingCharacter.h
 */

/**
 * @class IEvolvingCharacter
 * @author npc
 * @date 11/12/17
 * @brief An interface for pawns that need to register with the evolution system and save their ID.
 * 
 * You should register all EvolvingCharacters with the GameMode as spawnable character types.
 */
class SURVIVALGAME_API IEvolvingCharacter
{
	GENERATED_BODY()

private:
	FName DefaultName = FName(TEXT("Default"));

public:
	virtual void SetCID( int32 id, UPopulationManager* populationManager ) {}
	
    virtual int32 GetCID() { return 0; }

    virtual const FName& GetEvolutionPopulationTag() { return DefaultName; }
	
};
