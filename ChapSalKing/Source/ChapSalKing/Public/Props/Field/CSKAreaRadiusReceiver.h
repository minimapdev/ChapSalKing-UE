#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CSKAreaRadiusReceiver.generated.h"

UINTERFACE(MinimalAPI)
class UCSKAreaRadiusReceiver : public UInterface
{
	GENERATED_BODY()
};

// 범위형 스폰 문맥(떡 착지 반경 등)에서 반경을 주입받는 액터 — 미주입 시 자기 DA 반경 사용
class CHAPSALKING_API ICSKAreaRadiusReceiver
{
	GENERATED_BODY()

public:
	virtual void SetAreaRadiusOverride(float Radius) = 0;
};
