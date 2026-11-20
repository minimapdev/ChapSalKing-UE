#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CSKPlayerState.generated.h"

UCLASS()
class CHAPSALKING_API ACSKPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// 점수 = 몸집. 폰이 아니라 여기에 보관한다 — 폰 스폰/possess/리스폰 순서와 무관하게 읽히도록.
	// 값이 실제로 바뀔 때만 GameState에 순위표 갱신을 알린다
	void SetCSKScore(int32 NewScore);

	UFUNCTION(BlueprintPure, Category = "CSK|Score")
	int32 GetCSKScore() const
	{
		return CSKScore;
	}

private:
	UPROPERTY(Transient)
	int32 CSKScore = 0;
};
