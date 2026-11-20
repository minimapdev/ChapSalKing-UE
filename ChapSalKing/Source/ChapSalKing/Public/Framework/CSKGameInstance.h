#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Dough/DoughTypes.h"
#include "CSKGameInstance.generated.h"

class UCSKProjectSettingsData;

UCLASS()
class CHAPSALKING_API UCSKGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	static UCSKProjectSettingsData* GetProjectSettings(const UObject* WorldContextObject);

	static bool IsDrawDebugEnabled(const UObject* WorldContextObject);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CSK Data")
	TObjectPtr<UCSKProjectSettingsData> ProjectSettingsData;

	UFUNCTION(BlueprintCallable, Category = "CSK Data")
	UCSKProjectSettingsData* GetProjectSettingsData() const;

	// ======================================== Match Setup ===========================================
	// 레벨 전환을 넘어 살아남는 유일한 객체 — 로비에서 고른 값을 여기 담아 매치 레벨로 넘긴다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CSK Match Setup")
	EDoughCharacterType SelectedDoughType = EDoughCharacterType::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CSK Match Setup", meta = (ClampMin = "0"))
	int32 BotCount = 3;

	UFUNCTION(BlueprintCallable, Category = "CSK Match Setup")
	void SetSelectedDoughType(EDoughCharacterType NewType);

	// 0 ~ (최대 인원 - 1)로 잘린다
	UFUNCTION(BlueprintCallable, Category = "CSK Match Setup")
	void SetBotCount(int32 NewBotCount);

	// 한 판의 최대 인원 = 등록된 반죽 종류 수. 반죽은 중복 선택되지 않는다
	UFUNCTION(BlueprintPure, Category = "CSK Match Setup")
	int32 GetMaxPlayerCount() const;

	// 0번이 플레이어, 나머지가 봇. 로비 프리뷰와 실제 스폰이 같은 규칙을 쓰도록 여기 한 곳에 둔다
	UFUNCTION(BlueprintPure, Category = "CSK Match Setup")
	TArray<EDoughCharacterType> BuildMatchRoster() const;

	// 위 선택값을 확정하고 매치 레벨을 연다. 프론트엔드가 게임으로 넘어가는 유일한 지점
	UFUNCTION(BlueprintCallable, Category = "CSK Match Setup")
	void TravelToMatch();

private:
	// 첫 실행에서 한 번만 하드웨어 벤치마크를 돌려 그래픽 등급을 정한다 — 노트북은 낮게, 좋은 PC는 높게.
	// 결과는 GameUserSettings.ini에 저장되므로 다음 실행부터는 그 값을 그대로 쓴다
	void ApplyFirstRunScalability();
};
