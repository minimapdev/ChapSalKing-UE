#include "UI/Menu/CSKIntroBookData.h"

#if WITH_EDITOR

#include "Misc/DataValidation.h"

EDataValidationResult UCSKIntroBookData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!CoverArt)
	{
		Context.AddError(FText::FromString(TEXT("CoverArt가 비어있습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (Spreads.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("Spreads가 비어있습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	for (int32 Index = 0; Index < Spreads.Num(); ++Index)
	{
		if (!Spreads[Index].Art)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Spreads[%d].Art가 비어있습니다."), Index)));
			Result = EDataValidationResult::Invalid;
		}
	}

	if (!TurnSound)
	{
		Context.AddError(FText::FromString(TEXT("TurnSound가 비어있습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

#endif
