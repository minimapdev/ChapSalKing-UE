#include "UI/CSKCrosshairWidget.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"
#include "Player/Components/CSKPlayerShootComponent.h"

#include "Components/ProgressBar.h"

void UCSKCrosshairWidget::BindToPawn(ACSKPlayerCharacter* Character)
{
	if (UCSKPlayerShootComponent* ShootComponent = Character->GetShootComponent())
	{
		ShootComponent->OnShootStateChanged.AddDynamic(
			this, &UCSKCrosshairWidget::HandleShootStateChanged);

		SetAimingVisible(ShootComponent->IsAiming());
	}
	else
	{
		SetAimingVisible(false);
	}

	if (UCSKPlayerIngredientComponent* IngredientComponent = Character->GetIngredientComponent())
	{
		IngredientComponent->OnIngredientStateChanged.AddDynamic(
			this, &UCSKCrosshairWidget::HandleIngredientStateChanged);

		BP_OnIngredientChanged(IngredientComponent->GetCurrentIngredient());
	}

	RefreshCanShoot();
}

void UCSKCrosshairWidget::UnbindFromPawn(ACSKPlayerCharacter* Character)
{
	if (UCSKPlayerShootComponent* ShootComponent = Character->GetShootComponent())
	{
		ShootComponent->OnShootStateChanged.RemoveDynamic(
			this, &UCSKCrosshairWidget::HandleShootStateChanged);
	}

	if (UCSKPlayerIngredientComponent* IngredientComponent = Character->GetIngredientComponent())
	{
		IngredientComponent->OnIngredientStateChanged.RemoveDynamic(
			this, &UCSKCrosshairWidget::HandleIngredientStateChanged);
	}

	SetAimingVisible(false);
}

void UCSKCrosshairWidget::HandleShootStateChanged(bool bAiming, float CooldownRatio)
{
	SetAimingVisible(bAiming);
	RefreshCanShoot();
}

void UCSKCrosshairWidget::HandleIngredientStateChanged(UIngredientData* Ingredient, float GaugeRatio)
{
	BP_OnIngredientChanged(Ingredient);
}

void UCSKCrosshairWidget::SetAimingVisible(bool bAiming)
{
	SetVisibility(bAiming ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UCSKCrosshairWidget::RefreshCanShoot()
{
	const ACSKPlayerCharacter* Character = GetBoundCharacter();
	const UCSKPlayerShootComponent* ShootComponent = Character ? Character->GetShootComponent() : nullptr;

	const bool bCanShoot = ShootComponent && ShootComponent->IsShootReady();
	if (bCanShoot != bCanShootDisplayed)
	{
		bCanShootDisplayed = bCanShoot;
		BP_OnCanShootChanged(bCanShoot);
	}
}

void UCSKCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 조준 중일 때만 틱이 돈다 — 그 외에는 Collapsed라 여기 오지 않는다
	const ACSKPlayerCharacter* Character = GetBoundCharacter();
	const UCSKPlayerShootComponent* ShootComponent = Character ? Character->GetShootComponent() : nullptr;
	if (!ShootComponent)
	{
		return;
	}

	CooldownRing->SetPercent(ShootComponent->GetCooldownRatio());

	// 쿨다운이 끝나는 순간은 이벤트가 없다 — 조준 중에만 매 프레임 확인한다
	RefreshCanShoot();
}
