#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CSKPlayerComponentBase.generated.h"

class ACSKPlayerCharacter;
class UCSKProjectSettingsData;
class UCSKPlayerStatData;

UCLASS(Abstract)
class CHAPSALKING_API UCSKPlayerComponentBase : public UActorComponent
{
	GENERATED_BODY()

protected:
	bool CacheOwnerCSKCharacter();
	void InitStatData();

	UCSKProjectSettingsData* GetProjectSettings() const;
	UCSKPlayerStatData* GetPlayerStatData() const;

protected:
	UPROPERTY()
	TObjectPtr<ACSKPlayerCharacter> OwnerCSKCharacter;

	UPROPERTY()
	TObjectPtr<UCSKPlayerStatData> PlayerStatData;
};
