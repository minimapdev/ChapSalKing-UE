#include "Framework/CSKPouchDropZone.h"

#include "Components/SphereComponent.h"

ACSKPouchDropZone::ACSKPouchDropZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);
	ZoneSphere->SetSphereRadius(3000.0f);

	// 범위 표시일 뿐이다 — 무엇도 막지 않고 겹침도 알리지 않는다
	ZoneSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ZoneSphere->SetCollisionProfileName(TEXT("NoCollision"));
	ZoneSphere->SetHiddenInGame(true);
	ZoneSphere->ShapeColor = FColor(255, 180, 60);
}

float ACSKPouchDropZone::GetRadius() const
{
	// 스케일을 반영한 값이라, 뷰포트에서 액터를 키워도 그대로 따라온다
	return ZoneSphere->GetScaledSphereRadius();
}
