#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KingStatue.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class ACSKPlayerCharacter;

// 킹 재위 중 세워지는 순수 시각 연출용 동상. 킹 탄생 시 생성, 해제 시 제거.
UCLASS()
class CHAPSALKING_API AKingStatue : public AActor
{
	GENERATED_BODY()

public:
	AKingStatue();

	void InitializeFromKing(ACSKPlayerCharacter* King);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Statue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Statue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Statue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> CrownMesh;
};
