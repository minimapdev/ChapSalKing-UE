#include "Framework/CSKProjectSettingsData.h"

#if WITH_EDITOR

#include "Misc/DataValidation.h"
#include "Dough/DoughBlob.h"

EDataValidationResult UCSKProjectSettingsData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!DoughDataRegistry.Get())
	{
		Context.AddError(FText::FromString(TEXT("DoughDataRegistry가 비어있습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!PestleItemData.Get())
	{
		Context.AddError(FText::FromString(TEXT("PestleItemData가 비었습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!MortarZoneData.Get())
	{
		Context.AddError(FText::FromString(TEXT("MortarZoneData가 비었습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!SeesawData.Get())
	{
		Context.AddError(FText::FromString(TEXT("SeesawData가 비었습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!CrownData.Get())
	{
		Context.AddError(FText::FromString(TEXT("CrownData가 비었습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!PlayerStatData.Get())
	{
		Context.AddError(FText::FromString(TEXT("PlayerStatData가 비었습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!DoughBlobClass)
	{
		Context.AddError(FText::FromString(TEXT("DoughBlobClass가 비었습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (!DoughBlobData.Get())
	{
		Context.AddError(FText::FromString(TEXT("DoughBlobData가 비었습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	if (TteokDataMap.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("TteokDataMap이 비었습니다.")));
		Result = EDataValidationResult::Invalid;
	}
	for (const TPair<ETteokType, TObjectPtr<UTteokData>>& Pair : TteokDataMap)
	{
		if (!Pair.Value.Get())
		{
			Context.AddError(FText::FromString(FString::Printf(
				TEXT("TteokDataMap[%s]가 비었습니다."), *UEnum::GetValueAsString(Pair.Key))));
			Result = EDataValidationResult::Invalid;
		}
	}

	const ETteokType RequiredTypes[] =
	{
		ETteokType::Chapssal,
		ETteokType::Injeolmi,
		ETteokType::Kkul,
		ETteokType::Mujigae
	};

	for (const ETteokType RequiredType : RequiredTypes)
	{
		if (!TteokDataMap.Contains(RequiredType))
		{
			Context.AddError(FText::FromString(FString::Printf(
				TEXT("TteokDataMap에 %s 타입이 없습니다."), *UEnum::GetValueAsString(RequiredType))));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}

#endif
