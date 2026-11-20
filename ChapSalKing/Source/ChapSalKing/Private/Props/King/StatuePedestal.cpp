#include "Props/King/StatuePedestal.h"

#include "Props/King/KingStatue.h"
#include "Framework/CSKGameMode.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AStatuePedestal::AStatuePedestal()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PedestalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PedestalMesh"));
	PedestalMesh->SetupAttachment(SceneRoot);
	PedestalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PedestalMesh->SetCollisionObjectType(ECC_WorldStatic);
	PedestalMesh->SetCollisionResponseToAllChannels(ECR_Block);

	FigureSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("FigureSpawnPoint"));
	FigureSpawnPoint->SetupAttachment(PedestalMesh);
	FigureSpawnPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
}

void AStatuePedestal::BeginPlay()
{
	Super::BeginPlay();

	if (ACSKGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ACSKGameMode>() : nullptr)
	{
		GM->RegisterPedestal(this);
	}
}

void AStatuePedestal::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ACSKGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ACSKGameMode>() : nullptr)
	{
		GM->UnregisterPedestal(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AStatuePedestal::RaiseStatue(ACSKPlayerCharacter* King)
{
	if (CurrentStatue || !FigureClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(FigureSpawnPoint->GetComponentLocation());
	SpawnTransform.SetRotation(FigureSpawnPoint->GetComponentQuat());
	SpawnTransform.SetScale3D(FVector(StatueScale));

	CurrentStatue = World->SpawnActor<AKingStatue>(FigureClass, SpawnTransform, SpawnParams);
	if (CurrentStatue)
	{
		CurrentStatue->InitializeFromKing(King);
	}
}

void AStatuePedestal::ClearStatue()
{
	if (CurrentStatue)
	{
		CurrentStatue->Destroy();
		CurrentStatue = nullptr;
	}
}
