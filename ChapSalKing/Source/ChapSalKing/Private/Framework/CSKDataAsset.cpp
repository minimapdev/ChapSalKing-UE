#include "Framework/CSKDataAsset.h"

#if WITH_EDITOR

#include "Misc/DataValidation.h"

EDataValidationResult UCSKDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
}

#endif