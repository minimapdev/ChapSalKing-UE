#include "Player/CSKPlayerStatData.h"

#if WITH_EDITOR

#include "Misc/DataValidation.h"

EDataValidationResult UCSKPlayerStatData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	const FCSKPlayerScaleSettings& S = ScaleSettings;
	if (S.MinMassGrams > S.MaxMassGrams)
	{
		Context.AddError(FText::FromString(FString::Printf(
			TEXT("MinMassGrams(%.0f) > MaxMassGrams(%.0f)"), S.MinMassGrams, S.MaxMassGrams)));
		Result = EDataValidationResult::Invalid;
	}
	if (S.StartMassGrams < S.MinMassGrams || S.StartMassGrams > S.MaxMassGrams)
	{
		Context.AddError(FText::FromString(FString::Printf(
			TEXT("StartMassGrams(%.0f)가 Min~Max(%.0f~%.0f) 범위 밖입니다."),
			S.StartMassGrams, S.MinMassGrams, S.MaxMassGrams)));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

#endif
