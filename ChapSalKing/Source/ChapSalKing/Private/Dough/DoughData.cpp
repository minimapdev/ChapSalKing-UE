#include "Dough/DoughData.h"

#if WITH_EDITOR

#include "Misc/DataValidation.h"

EDataValidationResult UDoughData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (DisplayName.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("DisplayName이 비어있습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!EyeLTexture)
	{
		Context.AddError(FText::FromString(TEXT("EyeLTexture가 비어있습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!EyeRTexture)
	{
		Context.AddError(FText::FromString(TEXT("EyeRTexture가 비어있습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!MouthTexture)
	{
		Context.AddError(FText::FromString(TEXT("MouthTexture가 비어있습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!CheekTexture)
	{
		Context.AddWarning(FText::FromString(TEXT("CheekTexture가 비어있습니다.")));
	}

	return Result;
}

#endif