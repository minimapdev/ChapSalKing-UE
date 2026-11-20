#include "Props/Field/SmokeCloud.h"
#include "Props/Field/SmokeCloudData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerStatusComponent.h"

#include "EngineUtils.h"
#include "Components/SphereComponent.h"

ASmokeCloud::ASmokeCloud()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	PulseVolume = CreateDefaultSubobject<USphereComponent>(TEXT("PulseVolume"));
	PulseVolume->SetupAttachment(RootComponent);
	PulseVolume->InitSphereRadius(0.0f);
	PulseVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PulseVolume->SetCollisionObjectType(ECC_WorldDynamic);
	PulseVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	PulseVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PulseVolume->SetGenerateOverlapEvents(true);
}

void ASmokeCloud::BeginPlay()
{
	Super::BeginPlay();

	if (!CloudData)
	{
		UE_LOG(LogTemp, Warning, TEXT("SmokeCloud: CloudData is not assigned."));
		Destroy();
		return;
	}

	EffectiveRadius = AreaRadiusOverride > 0.0f ? AreaRadiusOverride : CloudData->BlockRadius;
	PulseVolume->SetSphereRadius(EffectiveRadius);
	SetLifeSpan(CloudData->LifeSeconds);
	BP_OnCloudSpawned(EffectiveRadius);

	// 직격 즉시 실명 + 이후 주기 펄스
	Pulse();
	GetWorldTimerManager().SetTimer(PulseTimerHandle, this, &ASmokeCloud::Pulse, CloudData->PulseInterval, true);
}

void ASmokeCloud::Pulse()
{
	TArray<AActor*> Overlapping;
	PulseVolume->GetOverlappingActors(Overlapping, ACSKPlayerCharacter::StaticClass());

	for (AActor* Actor : Overlapping)
	{
		ACSKPlayerCharacter* Character = Cast<ACSKPlayerCharacter>(Actor);
		if (!Character)
		{
			continue;
		}

		Character->GetStatusComponent()->TryApplyStatus(
			ECSKStatusType::VisionBlocked, CloudData->ObstructDuration, CloudData->CloudAffinity);
	}
}

bool ASmokeCloud::IsSightBlocked(UWorld* World, const FVector& From, const FVector& To, bool bKingConcealingOnly)
{
	for (TActorIterator<ASmokeCloud> It(World); It; ++It)
	{
		const ASmokeCloud* Cloud = *It;
		if (Cloud->EffectiveRadius <= 0.0f)
		{
			continue;
		}

		if (bKingConcealingOnly)
		{
			const ACSKPlayerCharacter* Caster = Cast<ACSKPlayerCharacter>(Cloud->GetOwner());
			if (!Caster || !Caster->IsChapSalKing())
			{
				continue;
			}
		}

		if (FMath::PointDistToSegment(Cloud->GetActorLocation(), From, To) < Cloud->EffectiveRadius)
		{
			return true;
		}
	}

	return false;
}
