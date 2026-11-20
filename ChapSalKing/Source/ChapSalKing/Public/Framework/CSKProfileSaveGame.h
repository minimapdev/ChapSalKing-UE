#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CSKProfileSaveGame.generated.h"

/**
 * 실행을 넘어 남아야 하는 플레이어 기록. 읽고 쓰는 건 UCSKProfileSubsystem만 한다.
 */
UCLASS()
class CHAPSALKING_API UCSKProfileSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 동화책 인트로를 끝까지 봤거나 건너뛰었는가 — 다음 실행부터는 타이틀로 바로 간다
	UPROPERTY()
	bool bIntroSeen = false;
};
