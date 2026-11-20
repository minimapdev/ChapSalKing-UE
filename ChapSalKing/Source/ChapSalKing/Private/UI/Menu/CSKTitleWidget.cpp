#include "UI/Menu/CSKTitleWidget.h"

#include "UI/Menu/CSKMenuRootWidget.h"

#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UCSKTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SingleButton->OnClicked.AddDynamic(this, &UCSKTitleWidget::HandleSingleClicked);
	MultiButton->OnClicked.AddDynamic(this, &UCSKTitleWidget::HandleMultiClicked);
	QuitButton->OnClicked.AddDynamic(this, &UCSKTitleWidget::HandleQuitClicked);
	ReplayIntroButton->OnClicked.AddDynamic(this, &UCSKTitleWidget::HandleReplayIntroClicked);
}

void UCSKTitleWidget::HandleSingleClicked()
{
	// 싱글은 봇으로 채워진 로비다 — 멀티와 같은 화면으로 간다
	GetMenuRoot()->ShowScreen(ECSKMenuScreen::Lobby);
}

void UCSKTitleWidget::HandleMultiClicked()
{
	GetMenuRoot()->ShowScreen(ECSKMenuScreen::RoomList);
}

void UCSKTitleWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UCSKTitleWidget::HandleReplayIntroClicked()
{
	GetMenuRoot()->TransitionToScreen(ECSKMenuScreen::Intro);
}
