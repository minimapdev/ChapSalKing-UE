#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StatuePedestal.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class AKingStatue;
class ACSKPlayerCharacter;

UCLASS()
class CHAPSALKING_API AStatuePedestal : public AActor
{
	GENERATED_BODY()

public:
	AStatuePedestal();

	void RaiseStatue(ACSKPlayerCharacter* King);

	void ClearStatue();

	UFUNCTION(BlueprintPure, Category = "CSK|Pedestal")
	AKingStatue* GetCurrentStatue() const
	{
		return CurrentStatue;
	}

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Pedestal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Pedestal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PedestalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Pedestal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> FigureSpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Pedestal", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AKingStatue> FigureClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Pedestal", meta = (AllowPrivateAccess = "true", ClampMin = "0.1", UIMin = "0.1"))
	float StatueScale = 1.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Pedestal")
	TObjectPtr<AKingStatue> CurrentStatue;
};
