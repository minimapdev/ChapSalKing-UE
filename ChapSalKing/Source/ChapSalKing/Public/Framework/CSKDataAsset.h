#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CSKDataAsset.generated.h"

/**
 * 찹쌀킹 프로젝트 DataAsset들의 공통 부모 클래스.
 * 공통 Data Validation 처리와 프로젝트 데이터 검색 기준으로 사용.
 */
UCLASS(Abstract, BlueprintType)
class CHAPSALKING_API UCSKDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public: 

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
