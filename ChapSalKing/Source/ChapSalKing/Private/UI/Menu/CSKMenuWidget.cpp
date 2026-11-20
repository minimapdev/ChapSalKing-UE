#include "UI/Menu/CSKMenuWidget.h"

#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "UI/CSKUIStyleData.h"
#include "UI/Menu/CSKMenuHUD.h"
#include "UI/Menu/CSKMenuRootWidget.h"

#include "GameFramework/PlayerController.h"

UCSKGameInstance* UCSKMenuWidget::GetCSKGameInstance() const
{
	return GetGameInstance<UCSKGameInstance>();
}

UCSKMenuRootWidget* UCSKMenuWidget::GetMenuRoot() const
{
	const APlayerController* PC = GetOwningPlayer();
	const ACSKMenuHUD* MenuHUD = PC ? PC->GetHUD<ACSKMenuHUD>() : nullptr;
	return MenuHUD ? MenuHUD->GetMenuRootWidget() : nullptr;
}

UCSKUIStyleData* UCSKMenuWidget::GetUIStyle() const
{
	UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	return Settings ? Settings->UISettings.UIStyleData : nullptr;
}
