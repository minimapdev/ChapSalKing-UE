#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Props/Field/CSKAreaRadiusReceiver.h"
#include "SmokeCloud.generated.h"

class USmokeCloudData;
class USphereComponent;

// 콩가루 연막: 봇 시야 차단 + 안의 캐릭터 실명 펄스 (같은 계열 면역).
// 시전자(Owner)가 킹이면 그 킹을 봇 위협 인지에서 숨김 (인절미 폭풍 은신)
UCLASS()
class CHAPSALKING_API ASmokeCloud : public AActor, public ICSKAreaRadiusReceiver
{
	GENERATED_BODY()

public:
	ASmokeCloud();

	virtual void SetAreaRadiusOverride(float Radius) override
	{
		AreaRadiusOverride = Radius;
	}

	// 시선(From→To)이 월드의 어떤 연막 구든 통과하면 true — 봇 시야 판정 단일 창구.
	// bKingConcealingOnly면 "킹이 시전한 연막"만 검사 (킹은 자기 연막 속에서만 은신)
	static bool IsSightBlocked(UWorld* World, const FVector& From, const FVector& To, bool bKingConcealingOnly = false);

protected:
	virtual void BeginPlay() override;

	// 연막 비주얼 훅 (나이아가라를 실제 반경에 맞추는 용도)
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Smoke")
	void BP_OnCloudSpawned(float BlockRadius);

private:
	void Pulse();

	UPROPERTY(EditAnywhere, Category = "Smoke")
	TObjectPtr<USmokeCloudData> CloudData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Smoke", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> PulseVolume;

	float AreaRadiusOverride = 0.0f;
	float EffectiveRadius = 0.0f;
	FTimerHandle PulseTimerHandle;
};
