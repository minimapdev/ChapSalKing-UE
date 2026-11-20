#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "Dough/DoughTypes.h"
#include "Engine/Texture2D.h"
#include "DoughData.generated.h"

/**
 * 각반죽 캐릭터의 데이터를 관리.
 */

UCLASS(BlueprintType)
class CHAPSALKING_API UDoughData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough")
	EDoughCharacterType CharacterType = EDoughCharacterType::White;

	// 화면에 노출되는 반죽 이름. 봇 플레이어 이름과 캐릭터 선택 슬롯 이름표로 쓰인다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough")
	FText DisplayName;

	// 반죽 선택 패널에 뜨는 한 줄 소개
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough")
	FText Description;

	// ======================================== Dough Visual ===========================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough Visual|Body")
	FLinearColor BodyColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough Visual|VFX")
	FLinearColor BodySlamInnerColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough Visual|VFX")
	FLinearColor BodySlamOuterColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough Visual|Face")
	TObjectPtr<UTexture2D> EyeLTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough Visual|Face")
	TObjectPtr<UTexture2D> EyeRTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough Visual|Face")
	TObjectPtr<UTexture2D> MouthTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough Visual|Face")
	FLinearColor MouthTintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough Visual|Face")
	TObjectPtr<UTexture2D> CheekTexture;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
