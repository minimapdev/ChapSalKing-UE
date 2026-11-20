#include "UI/Menu/CSKMenuHUD.h"

#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "UI/Menu/CSKMenuRootWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"

void ACSKMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!Settings)
	{
		return;
	}

	const TSubclassOf<UCSKMenuRootWidget> WidgetClass = Settings->UISettings.MenuRootWidgetClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CSK] ACSKMenuHUD: UISettings.MenuRootWidgetClass 미설정 — 프론트엔드가 표시되지 않습니다"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();

	MenuRootWidget = CreateWidget<UCSKMenuRootWidget>(PC, WidgetClass);
	if (!MenuRootWidget)
	{
		return;
	}

	MenuRootWidget->AddToViewport();

	// 프론트엔드에는 조작할 폰이 없다 — 입력은 전부 위젯이 가져간다
	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PC, MenuRootWidget);
	PC->SetShowMouseCursor(true);
}

void ACSKMenuHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MenuRootWidget)
	{
		MenuRootWidget->RemoveFromParent();
		MenuRootWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
