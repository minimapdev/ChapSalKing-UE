#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PestleSpawnPoint.generated.h"

class UArrowComponent;

UCLASS()
class CHAPSALKING_API APestleSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	APestleSpawnPoint();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|PestleSpawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|PestleSpawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> EditorArrow;
#endif
};
