#include "Props/King/KingStatue.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerVisualComponent.h"

#include "Components/SkeletalMeshComponent.h"

AKingStatue::AKingStatue()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(SceneRoot);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CrownMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CrownMesh"));
	CrownMesh->SetupAttachment(BodyMesh);
	CrownMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AKingStatue::InitializeFromKing(ACSKPlayerCharacter* King)
{
	// 동상 외형(메시·머티리얼·크라운)은 전부 BP_KingStatue에 고정으로 세팅됨.
	// king마다 다른 건 눈/입/볼 텍스처뿐 → DoughData에서 얼굴 텍스처만 받아 얼굴 슬롯에 적용.
	if (King)
	{
		UCSKPlayerVisualComponent::ApplyDoughVisualToMesh(King->GetPlayerDoughData(), BodyMesh);
	}
}
