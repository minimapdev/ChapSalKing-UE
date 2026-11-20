#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Dough/DoughTypes.h"
#include "CSKGameMode.generated.h"

class APestleItem;
class ALuckyPouch;
class ACSKPouchDropZone;
class ACSKPlayerCharacter;
class UIngredientData;
struct FCSKPouchDropSettings;
class APlayerState;
class AStatuePedestal;

UCLASS()
class CHAPSALKING_API ACSKGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACSKGameMode();

	void RegisterPedestal(AStatuePedestal* Pedestal);
	void UnregisterPedestal(AStatuePedestal* Pedestal);
	void TryEndMatchFromKingCapture(ACSKPlayerCharacter* KingCharacter);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 폰의 BeginPlay가 DoughType을 읽으므로, 스폰을 지연시켜 그 전에 선택값을 꽂는다
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(
		AController* NewPlayer, const FTransform& SpawnTransform) override;

private:
	ACSKPlayerCharacter* SpawnDoughPawn(
		UClass* PawnClass, const FTransform& SpawnTransform, EDoughCharacterType DoughType);
	void SpawnBots();

	void StartMatch();
	void EndMatch();
	void FinalizeMatch(APlayerState* ForcedWinner);
	bool AreAllOpponentsStuckToKing(const ACSKPlayerCharacter* KingCharacter) const;

	void HandleChapSalKingChanged(ACSKPlayerCharacter* NewKing);
	APestleItem* FindPestleInWorld() const;

	// 매치 시작·킹 종료 때 부른다. SpawnCountdown 뒤에 복주머니 여러 개가 동시에 착지하고
	// 그중 하나에만 절구공이가 들어 있다 — 어느 것인지는 아무도 모른다
	void ScheduleNextPestleWave();
	void DropPestleWave();

	// 매치 내내 PouchInterval마다 하나씩. 지상 재료가 없으므로 이 주기가 곧 보급의 전부다
	void DropSinglePouch();

	// 존 안 네비메시 위의 임의 지점. 고정 낙하 지점은 두지 않는다.
	// TakenLocations에 이미 잡힌 자리를 넘기면 MinSeparation만큼 떨어진 곳만 돌려준다
	bool FindDropLocation(
		const FCSKPouchDropSettings& DropSettings,
		const TArray<FVector>& TakenLocations,
		float MinSeparation,
		FVector& OutLocation) const;

	ACSKPouchDropZone* PickRandomDropZone() const;

	// 가중치 뽑기. 절구공이는 여기 없다 — 웨이브가 확정 배치한다
	void RollLoot(const FCSKPouchDropSettings& DropSettings, UIngredientData*& OutIngredient, bool& bOutIsBomb) const;

	ALuckyPouch* SpawnPouchAt(
		const FCSKPouchDropSettings& DropSettings,
		const FVector& LandingLocation,
		APestleItem* PestleToHide);

	UPROPERTY()
	TArray<TObjectPtr<AStatuePedestal>> Pedestals;

	FDelegateHandle ChapSalKingChangedHandle;
	FTimerHandle MatchTimerHandle;
	FTimerHandle PestleWaveTimerHandle;
	FTimerHandle PouchIntervalTimerHandle;
};
