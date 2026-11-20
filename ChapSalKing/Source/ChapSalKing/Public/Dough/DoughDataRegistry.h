#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "Dough/DoughData.h"
#include "DoughDataRegistry.generated.h"

/**
 * 반죽 캐릭터 데이터 목록과 공통 플레이 수치를 관리.
 */
UCLASS(BlueprintType)
class CHAPSALKING_API UDoughDataRegistry : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough")
	TArray<TObjectPtr<UDoughData>> DoughDataList;

	UFUNCTION(BlueprintCallable, Category = "Dough")
	UDoughData* FindByType(EDoughCharacterType CharacterType) const;

	// 등록된 반죽 종류 = 한 판의 최대 인원. 반죽은 중복 선택되지 않는다
	UFUNCTION(BlueprintCallable, Category = "Dough")
	TArray<EDoughCharacterType> GetAllCharacterTypes() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
