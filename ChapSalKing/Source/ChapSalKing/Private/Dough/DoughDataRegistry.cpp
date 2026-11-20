#include "Dough/DoughDataRegistry.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UDoughData* UDoughDataRegistry::FindByType(EDoughCharacterType CharacterType) const
{
	for (UDoughData* Data : DoughDataList)
	{
		if (Data && Data->CharacterType == CharacterType)
		{
			return Data;
		}
	}

	return nullptr;
}

TArray<EDoughCharacterType> UDoughDataRegistry::GetAllCharacterTypes() const
{
	TArray<EDoughCharacterType> Types;
	Types.Reserve(DoughDataList.Num());

	for (const UDoughData* Data : DoughDataList)
	{
		if (Data)
		{
			Types.AddUnique(Data->CharacterType);
		}
	}

	return Types;
}

#if WITH_EDITOR
EDataValidationResult UDoughDataRegistry::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (DoughDataList.Num() == 0)
	{
		Context.AddError(FText::FromString(TEXT("DoughDataList가 비어있습니다.")));
		Result = EDataValidationResult::Invalid;
	}

	TSet<EDoughCharacterType> SeenTypes;
	for (UDoughData* Data : DoughDataList)
	{
		if (!Data)
		{
			Context.AddError(FText::FromString(TEXT("DoughDataList에 비어있는 항목이 있습니다.")));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		// CharacterType이 곧 슬롯 하나 — 중복되면 최대 인원이 조용히 줄어든다
		bool bAlreadySeen = false;
		SeenTypes.Add(Data->CharacterType, &bAlreadySeen);
		if (bAlreadySeen)
		{
			Context.AddError(FText::FromString(TEXT("DoughDataList에 CharacterType이 중복된 항목이 있습니다.")));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif
