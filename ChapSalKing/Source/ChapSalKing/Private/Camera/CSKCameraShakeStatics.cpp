#include "Camera/CSKCameraShakeStatics.h"

#include "Camera/CSKCameraShakeSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UCameraShakeBase* UCSKCameraShakeStatics::PlayCameraShake(
	const UObject* WorldContextObject,
	FGameplayTag ShakeTag,
	float Scale)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	ULocalPlayer* LocalPlayer = PlayerController
		? PlayerController->GetLocalPlayer()
		: nullptr;
	UCSKCameraShakeSubsystem* Subsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UCSKCameraShakeSubsystem>()
		: nullptr;

	return Subsystem
		? Subsystem->PlayShake(ShakeTag, Scale)
		: nullptr;
}

UCameraShakeBase* UCSKCameraShakeStatics::PlayCameraShakeAtLocation(
	const UObject* WorldContextObject,
	FGameplayTag ShakeTag,
	FVector SourceLocation,
	float Scale)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	ULocalPlayer* LocalPlayer = PlayerController
		? PlayerController->GetLocalPlayer()
		: nullptr;
	UCSKCameraShakeSubsystem* Subsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UCSKCameraShakeSubsystem>()
		: nullptr;

	return Subsystem
		? Subsystem->PlayShakeAtLocation(ShakeTag, SourceLocation, Scale)
		: nullptr;
}

void UCSKCameraShakeStatics::StopCameraShake(
	const UObject* WorldContextObject,
	UCameraShakeBase* ShakeInstance,
	bool bImmediately)
{
	if (!WorldContextObject || !ShakeInstance)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	ULocalPlayer* LocalPlayer = PlayerController
		? PlayerController->GetLocalPlayer()
		: nullptr;
	UCSKCameraShakeSubsystem* Subsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UCSKCameraShakeSubsystem>()
		: nullptr;

	if (Subsystem)
	{
		Subsystem->StopShake(ShakeInstance, bImmediately);
	}
}
