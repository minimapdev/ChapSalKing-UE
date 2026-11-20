#include "UI/Menu/CSKLobbyWidget.h"

#include "Dough/DoughData.h"
#include "Dough/DoughDataRegistry.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKMenuGameMode.h"
#include "Framework/CSKProjectSettingsData.h"
#include "UI/Menu/CSKDoughProfileButtonWidget.h"
#include "UI/Menu/CSKLobbySlotWidget.h"
#include "UI/Menu/CSKMenuRootWidget.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Engine/World.h"

void UCSKLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BotCountDecreaseButton->OnClicked.AddDynamic(this, &UCSKLobbyWidget::HandleBotCountDecrease);
	BotCountIncreaseButton->OnClicked.AddDynamic(this, &UCSKLobbyWidget::HandleBotCountIncrease);
	StartButton->OnClicked.AddDynamic(this, &UCSKLobbyWidget::HandleStartClicked);
	BackButton->OnClicked.AddDynamic(this, &UCSKLobbyWidget::HandleBackClicked);

	BuildProfileButtons();
	BuildSlotWidgets();
	Refresh();
}

void UCSKLobbyWidget::NativeOnScreenActivated()
{
	bStageActive = true;

	// 화면 밖에서 봇 인원이나 선택이 바뀌어 있을 수 있다
	Refresh();

	GetWorld()->GetAuthGameMode<ACSKMenuGameMode>()->SetLobbyPreviewVisible(true);
}

void UCSKLobbyWidget::NativeOnScreenDeactivated()
{
	bStageActive = false;

	GetWorld()->GetAuthGameMode<ACSKMenuGameMode>()->SetLobbyPreviewVisible(false);
}

void UCSKLobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bStageActive)
	{
		return;
	}

	TArray<FCSKLobbySlotViewportRect> SlotRects;
	SlotRects.Reserve(SlotWidgets.Num());

	for (const UCSKLobbySlotWidget* SlotWidget : SlotWidgets)
	{
		// 칸 전체가 아니라 카드가 실제로 놓이는 자리다 — 칸이 카드보다 넓어도
		// 3D 카드가 늘어나지 않는다
		const FGeometry& SlotGeometry = SlotWidget->GetCardGeometry();
		const FVector2D LocalSize = SlotGeometry.GetLocalSize();

		// 칸의 중심과 아래·오른쪽 끝을 픽셀 좌표로 옮긴다. 로컬 크기를 그냥 쓰면 DPI 스케일이
		// 빠져 고해상도에서 반죽만 작아진다 — 가로·세로도 두 점의 픽셀 차이로 잰다
		FVector2D CenterPixel, CenterViewport;
		FVector2D BottomPixel, BottomViewport;
		FVector2D RightPixel, RightViewport;
		USlateBlueprintLibrary::LocalToViewport(
			this, SlotGeometry, 0.5f * LocalSize, CenterPixel, CenterViewport);
		USlateBlueprintLibrary::LocalToViewport(
			this, SlotGeometry, FVector2D(0.5f * LocalSize.X, LocalSize.Y), BottomPixel, BottomViewport);
		USlateBlueprintLibrary::LocalToViewport(
			this, SlotGeometry, FVector2D(LocalSize.X, 0.5f * LocalSize.Y), RightPixel, RightViewport);

		FCSKLobbySlotViewportRect SlotRect;
		SlotRect.Center = CenterPixel;
		SlotRect.Height = 2.0f * (BottomPixel.Y - CenterPixel.Y);
		SlotRect.Width = 2.0f * (RightPixel.X - CenterPixel.X);
		SlotRects.Add(SlotRect);
	}

	GetWorld()->GetAuthGameMode<ACSKMenuGameMode>()->AlignLobbyPreview(GetOwningPlayer(), SlotRects);
}

void UCSKLobbyWidget::BuildProfileButtons()
{
	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	UDoughDataRegistry* Registry = Settings ? Settings->DoughDataRegistry : nullptr;
	if (!Registry)
	{
		return;
	}

	if (!ProfileButtonClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CSK] LobbyWidget: ProfileButtonClass가 비었습니다 — WBP_CSKLobby 디폴트에서 지정하세요"));
		return;
	}

	for (UDoughData* DoughData : Registry->DoughDataList)
	{
		if (!DoughData)
		{
			continue;
		}

		UCSKDoughProfileButtonWidget* ProfileButton =
			CreateWidget<UCSKDoughProfileButtonWidget>(GetOwningPlayer(), ProfileButtonClass);

		ProfilePanel->AddChild(ProfileButton);
		ProfileButton->InitProfile(DoughData, this);
		ProfileButtons.Add(ProfileButton);
	}
}

void UCSKLobbyWidget::BuildSlotWidgets()
{
	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!Settings)
	{
		return;
	}

	if (!SlotWidgetClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CSK] LobbyWidget: SlotWidgetClass가 비었습니다 — WBP_CSKLobby 디폴트에서 지정하세요"));
		return;
	}

	// 3D 프리뷰 격자와 같은 설정을 읽는다 — 칸 수도 칸 순서도 어긋날 수 없다
	const FCSKLobbySettings& LobbySettings = Settings->LobbySettings;
	const int32 SlotCount = LobbySettings.Columns * LobbySettings.Rows;
	const int32 MaxPlayerCount = GetCSKGameInstance()->GetMaxPlayerCount();

	if (SlotCount < MaxPlayerCount)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CSK] LobbyWidget: 슬롯이 %d칸뿐인데 최대 인원은 %d명입니다 — LobbySettings의 Columns/Rows를 늘리세요"),
			SlotCount, MaxPlayerCount);
	}

	for (int32 Index = 0; Index < SlotCount; ++Index)
	{
		UCSKLobbySlotWidget* SlotWidget =
			CreateWidget<UCSKLobbySlotWidget>(GetOwningPlayer(), SlotWidgetClass);

		// UniformGridPanel은 행·열을 지정하지 않으면 자식이 전부 (0,0)에 겹쳐 쌓인다
		UUniformGridSlot* GridSlot = SlotPanel->AddChildToUniformGrid(
			SlotWidget, Index / LobbySettings.Columns, Index % LobbySettings.Columns);

		// 칸을 꽉 채워야 뒤에 깔린 프리뷰의 칸 경계와 맞는다
		GridSlot->SetHorizontalAlignment(HAlign_Fill);
		GridSlot->SetVerticalAlignment(VAlign_Fill);

		SlotWidgets.Add(SlotWidget);
	}
}

void UCSKLobbyWidget::SelectDough(EDoughCharacterType NewType)
{
	GetCSKGameInstance()->SetSelectedDoughType(NewType);
	Refresh();
}

void UCSKLobbyWidget::Refresh()
{
	UCSKGameInstance* GameInstance = GetCSKGameInstance();

	for (UCSKDoughProfileButtonWidget* ProfileButton : ProfileButtons)
	{
		ProfileButton->SetSelected(
			ProfileButton->GetCharacterType() == GameInstance->SelectedDoughType);
	}

	BotCountText->SetText(FText::AsNumber(GameInstance->BotCount));

	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	UDoughDataRegistry* Registry = Settings ? Settings->DoughDataRegistry : nullptr;
	ACSKMenuGameMode* MenuGameMode = GetWorld()->GetAuthGameMode<ACSKMenuGameMode>();
	if (!Registry || !MenuGameMode)
	{
		return;
	}

	const UDoughData* SelectedDough = Registry->FindByType(GameInstance->SelectedDoughType);
	SelectedNameText->SetText(SelectedDough->DisplayName);
	SelectedDescText->SetText(SelectedDough->Description);

	const TArray<EDoughCharacterType> Roster = GameInstance->BuildMatchRoster();

	// 칸 자리에 선 3D 반죽과 그 위에 얹힌 슬롯 UI가 같은 명단을 본다
	MenuGameMode->ApplyLobbyRoster(Roster);

	for (int32 Index = 0; Index < SlotWidgets.Num(); ++Index)
	{
		UDoughData* SlotDough = Roster.IsValidIndex(Index)
			? Registry->FindByType(Roster[Index])
			: nullptr;

		// 0번은 언제나 플레이어 자리다
		SlotWidgets[Index]->SetSlotData(SlotDough, Index == 0);
	}
}

void UCSKLobbyWidget::HandleBotCountDecrease()
{
	UCSKGameInstance* GameInstance = GetCSKGameInstance();
	GameInstance->SetBotCount(GameInstance->BotCount - 1);
	Refresh();
}

void UCSKLobbyWidget::HandleBotCountIncrease()
{
	UCSKGameInstance* GameInstance = GetCSKGameInstance();
	GameInstance->SetBotCount(GameInstance->BotCount + 1);
	Refresh();
}

void UCSKLobbyWidget::HandleStartClicked()
{
	GetCSKGameInstance()->TravelToMatch();
}

void UCSKLobbyWidget::HandleBackClicked()
{
	GetMenuRoot()->ShowScreen(ECSKMenuScreen::Title);
}
