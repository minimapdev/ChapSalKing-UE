#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSKPouchDropZone.generated.h"

class USphereComponent;

/**
 * 복주머니가 떨어질 수 있는 영역. 레벨에 놓고 옮기고 반경을 늘리는 것만으로 낙하 범위가 정해진다.
 *
 * 범위는 ZoneSphere의 반경 하나가 전부다 — 액터에 반경 프로퍼티를 따로 두면 둘이 어긋나고,
 * 뷰포트에서 만진 값이 되돌아간다. 뷰포트의 반경 핸들을 끌거나 Details의
 * ZoneSphere → Sphere Radius를 고치면 되고, 액터 스케일도 함께 반영된다.
 *
 * 낙하 지점을 미리 찍어 두지 않는 이유: 고정 지점은 플레이어가 몇 곳만 외우면 되어 금세
 * 지루해지고, 레벨을 고칠 때마다 지점을 다시 앉혀야 한다. 존 안의 네비메시 위 임의 지점을
 * 매번 새로 뽑으면 둘 다 없다.
 *
 * 레벨에 여러 개 놓을 수 있다. 낙하마다 존을 하나 골라 그 안에서 지점을 뽑으므로,
 * 맵을 구역으로 나눈 뒤에는 구역마다 하나씩 놓으면 코드를 고치지 않고 구역 배분이 된다.
 *
 * 하나도 없으면 아레나 전체를 쓴다 — ACSKArenaBounds의 벽이 네비게이션을 막으므로
 * 네비메시 자체가 이미 아레나 안쪽뿐이다.
 */
UCLASS()
class CHAPSALKING_API ACSKPouchDropZone : public AActor
{
	GENERATED_BODY()

public:
	ACSKPouchDropZone();

	// 이 반경 안의 네비메시 위에만 떨어진다
	float GetRadius() const;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|PouchDropZone",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> ZoneSphere;
};
