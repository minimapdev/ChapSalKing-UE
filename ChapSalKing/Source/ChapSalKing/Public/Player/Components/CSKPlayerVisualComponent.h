#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CSKPlayerVisualComponent.generated.h"

/**
 * Player의 Visual을 담당하는 컴포넌트
 */

class UDoughData;
class USkeletalMeshComponent;
class UIngredientData;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerVisualComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCSKPlayerVisualComponent();

public:
	UFUNCTION(BlueprintCallable, Category = "CSK|Visual")
	void ApplyVisualData(UDoughData* DoughData);

	// 반죽 외형(반죽색/눈·입·볼 MID)을 임의의 스켈레탈 메시에 적용. 캐릭터/동상 공용.
	static void ApplyDoughVisualToMesh(UDoughData* DoughData, USkeletalMeshComponent* MeshComp);

	// 킹 떡 코팅 적용/해제 (게이지 없이 상시, 슬롯을 점유해 재료 갱신을 무시). nullptr이면 해제.
	void SetTteokBodyOverlay(UMaterialInterface* Material);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleIngredientStateChanged(UIngredientData* Ingredient, float GaugeRatio);

	void SetBodyOverlay(UMaterialInterface* Material);

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyOverlayMID;

	// 현재 슬롯에 적용된 애셋 (동일 애셋 재요청 시 MID 재생성 방지)
	UPROPERTY()
	TObjectPtr<UMaterialInterface> AppliedBodyOverlay;

	// 킹 떡 코팅이 슬롯 점유 중 — true면 재료 상태 변경 무시
	bool bTteokOverride = false;
};
