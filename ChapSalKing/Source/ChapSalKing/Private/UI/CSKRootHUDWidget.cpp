#include "UI/CSKRootHUDWidget.h"

#include "GameFramework/PlayerController.h"

void UCSKRootHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	PlayerController->OnPossessedPawnChanged.AddDynamic(
		this, &UCSKRootHUDWidget::HandlePossessedPawnChanged);

	// 위젯 생성 시점에 이미 폰이 있으면 바로 전파
	RebindPawn(PlayerController->GetPawn());
}

void UCSKRootHUDWidget::NativeDestruct()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->OnPossessedPawnChanged.RemoveDynamic(
			this, &UCSKRootHUDWidget::HandlePossessedPawnChanged);
	}

	Super::NativeDestruct();
}

void UCSKRootHUDWidget::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	RebindPawn(NewPawn);
}
