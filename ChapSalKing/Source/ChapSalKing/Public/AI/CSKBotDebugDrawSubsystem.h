#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CSKBotDebugDrawSubsystem.generated.h"

class UCanvas;
class APlayerController;

UCLASS()
class CHAPSALKING_API UCSKBotDebugDrawSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void Draw(UCanvas* Canvas, APlayerController* PlayerController);

	FDelegateHandle DrawHandle;
};
