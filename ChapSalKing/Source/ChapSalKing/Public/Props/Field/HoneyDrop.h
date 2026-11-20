#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HoneyDrop.generated.h"

class UStaticMeshComponent;
class UHoneyDropData;

UCLASS()
class CHAPSALKING_API AHoneyDrop : public AActor
{
	GENERATED_BODY()

public:
	AHoneyDrop();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Drop")
	void BP_OnDropLanded(const FVector& Location);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drop", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, Category = "Drop")
	TObjectPtr<UHoneyDropData> DropData;
};
