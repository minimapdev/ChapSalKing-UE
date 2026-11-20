#pragma once

#include "CoreMinimal.h"
#include "CSKCombatRequestTypes.generated.h"

UENUM(BlueprintType)
enum class ECSKCombatActionType : uint8
{
	None,
	BasicAttack		
};

USTRUCT(BlueprintType)
struct FCSKCombatRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "CSK|Combat")
	ECSKCombatActionType ActionType = ECSKCombatActionType::None;
};
