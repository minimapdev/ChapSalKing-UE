#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CSKProfileSubsystem.generated.h"

class UCSKProfileSaveGame;

/**
 * 플레이어 기록(UCSKProfileSaveGame)의 단일 소유자.
 * 게임 인스턴스와 수명이 같아 레벨이 바뀌어도 한 번 읽은 기록을 그대로 쓴다.
 * 위젯은 슬롯 이름이나 저장 시점을 몰라도 되도록 질의·기록 함수만 쓴다.
 */
UCLASS()
class CHAPSALKING_API UCSKProfileSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "CSK|Profile")
	bool HasSeenIntro() const;

	// 기록하고 바로 저장한다
	UFUNCTION(BlueprintCallable, Category = "CSK|Profile")
	void MarkIntroSeen();

private:
	void Save() const;

	UPROPERTY(Transient)
	TObjectPtr<UCSKProfileSaveGame> Profile;
};
