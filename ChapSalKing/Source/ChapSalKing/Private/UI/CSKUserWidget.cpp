#include "UI/CSKUserWidget.h"

#include "Framework/CSKGameState.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKPlayerState.h"
#include "Framework/CSKProjectSettingsData.h"
#include "Player/CSKPlayerCharacter.h"
#include "UI/CSKUIStyleData.h"

#include "Blueprint/WidgetTree.h"
#include "Engine/World.h"

ACSKGameState* UCSKUserWidget::GetCSKGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ACSKGameState>() : nullptr;
}

ACSKPlayerState* UCSKUserWidget::GetOwningCSKPlayerState() const
{
	return GetOwningPlayerState<ACSKPlayerState>();
}

ACSKPlayerCharacter* UCSKUserWidget::GetBoundCharacter() const
{
	return BoundCharacter.Get();
}

UCSKUIStyleData* UCSKUserWidget::GetUIStyle() const
{
	UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	return Settings ? Settings->UISettings.UIStyleData : nullptr;
}

void UCSKUserWidget::RebindPawn(APawn* NewPawn)
{
	ACSKPlayerCharacter* NewCharacter = Cast<ACSKPlayerCharacter>(NewPawn);
	ACSKPlayerCharacter* OldCharacter = BoundCharacter.Get();

	if (OldCharacter == NewCharacter)
	{
		return;
	}

	if (OldCharacter)
	{
		UnbindFromPawn(OldCharacter);
	}

	BoundCharacter = NewCharacter;

	if (NewCharacter)
	{
		BindToPawn(NewCharacter);
	}

	// 자기 위젯 트리 안의 CSK 자식에게 이어서 전파 — 중첩 깊이에 상관없이 닿는다
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([NewPawn](UWidget* Widget)
			{
				if (UCSKUserWidget* ChildWidget = Cast<UCSKUserWidget>(Widget))
				{
					ChildWidget->RebindPawn(NewPawn);
				}
			});
	}
}

void UCSKUserWidget::NativeDestruct()
{
	if (ACSKPlayerCharacter* Character = BoundCharacter.Get())
	{
		UnbindFromPawn(Character);
	}
	BoundCharacter = nullptr;

	Super::NativeDestruct();
}
