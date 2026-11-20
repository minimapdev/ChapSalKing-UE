#include "UI/CSKHUD.h"

#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "UI/CSKRootHUDWidget.h"

#include "Blueprint/UserWidget.h"

void ACSKHUD::BeginPlay()
{
	Super::BeginPlay();

	UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!Settings)
	{
		return;
	}

	const TSubclassOf<UCSKRootHUDWidget> WidgetClass = Settings->UISettings.RootHUDWidgetClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CSK] ACSKHUD: UISettings.RootHUDWidgetClass 미설정 — HUD가 표시되지 않습니다"));
		return;
	}

	RootHUDWidget = CreateWidget<UCSKRootHUDWidget>(GetOwningPlayerController(), WidgetClass);
	if (RootHUDWidget)
	{
		RootHUDWidget->AddToViewport();
	}
}

void ACSKHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (RootHUDWidget)
	{
		RootHUDWidget->RemoveFromParent();
		RootHUDWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
