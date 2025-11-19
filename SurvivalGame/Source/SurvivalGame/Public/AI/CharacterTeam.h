#pragma once

#include "CharacterTeam.generated.h"

/**
 * @file CharacterTeam.h
 */

/**
 * @author npc
 * @brief The possible teams a character can be on.
 * 
 */

UENUM()
enum class ECharacterTeam : uint8
{
    Humans,
    Zombies
};