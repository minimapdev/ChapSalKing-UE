#pragma once

#include "CoreMinimal.h"
#include "Props/Mortar/CSKCarryablePhysicsItemData.h"
#include "PestleItemData.generated.h"

class UNiagaraSystem;
class UMaterialInterface;

UCLASS(BlueprintType)
class CHAPSALKING_API UPestleItemData : public UCSKCarryablePhysicsItemData
{
	GENERATED_BODY()

public:
	// 스폰 카운트다운 동안 착지 지점에 재생되는 예고 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Spawn")
	TObjectPtr<UNiagaraSystem> DescentEffect;

	// 하강 예고 이펙트의 전체 배율. 나이아가라 컴포넌트 스케일로 적용된다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Spawn", meta = (ClampMin = "0.01"))
	float DescentEffectScale = 1.0f;

	// 절구공이 위치 알림용 상시 발광 이펙트. 절구에 꽂혀 있거나 하강 중에는 꺼짐
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual")
	TObjectPtr<UNiagaraSystem> BeaconEffect;

	// 알림용 메시 오버레이 머티리얼. BeaconEffect와 같은 주기로 켜지고 꺼짐
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual")
	TObjectPtr<UMaterialInterface> BeaconOverlayMaterial;
};
