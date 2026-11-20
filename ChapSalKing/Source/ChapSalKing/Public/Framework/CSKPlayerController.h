#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Framework/CSKGameState.h"
#include "CSKPlayerController.generated.h"

class ACSKPlayerCharacter;
class ACSKGameState;

UCLASS()
class CHAPSALKING_API ACSKPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|UI")
	void BP_OnChapSalKingAppeared(ACSKPlayerCharacter* King);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|UI")
	void BP_OnChapSalKingCleared();

private:
	void HandleChapSalKingChanged(ACSKPlayerCharacter* NewKing);

	UFUNCTION()
	void HandleMatchPhaseChanged(ECSKMatchPhase NewPhase);

	void EnterMatchResultInputMode();

	ACSKGameState* GetCSKGameState() const;

	FDelegateHandle ChapSalKingChangedHandle;
};
