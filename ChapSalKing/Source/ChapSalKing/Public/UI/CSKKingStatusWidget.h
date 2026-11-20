#pragma once

#include "CoreMinimal.h"
#include "UI/CSKUserWidget.h"
#include "CSKKingStatusWidget.generated.h"

class UTextBlock;
class UProgressBar;
class ACSKPlayerCharacter;
class UTteokData;

/**
 * 찹쌀킹 상태. 킹이 있을 때만 보인다.
 * 내가 킹인지 아닌지는 BP_OnKingAppeared의 bIsLocalPlayer로 분기한다.
 *
 * 요구 위젯: KingNameText (TextBlock), SessionBar (ProgressBar)
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKKingStatusWidget : public UCSKUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 세션 잔여 비율이 이 값 아래로 내려가면 BP_OnUrgentChanged(true)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|King", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UrgentRatio = 0.2f;

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|King")
	void BP_OnKingAppeared(ACSKPlayerCharacter* King, bool bIsLocalPlayer, UTteokData* TteokData);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|King")
	void BP_OnKingCleared();

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|King")
	void BP_OnUrgentChanged(bool bInUrgent);

	UPROPERTY(BlueprintReadOnly, Category = "CSK|King", meta = (BindWidget))
	TObjectPtr<UTextBlock> KingNameText;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|King", meta = (BindWidget))
	TObjectPtr<UProgressBar> SessionBar;

private:
	UFUNCTION()
	void HandleChapSalKingChanged(ACSKPlayerCharacter* NewKing);

	void ApplyKing(ACSKPlayerCharacter* NewKing);

	bool bUrgentDisplayed = false;
};
