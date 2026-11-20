#include "Player/Status/CSKStatusVFXData.h"

UCSKStatusVFXCatalog::UCSKStatusVFXCatalog()
{
	EnsureDefaultEntries();
}

void UCSKStatusVFXCatalog::PostLoad()
{
	Super::PostLoad();
	EnsureDefaultEntries();
}

void UCSKStatusVFXCatalog::EnsureDefaultEntries()
{
	auto AddDefault = [this](ECSKStatusVisualType Type, const FVector& Location)
	{
		if (!Effects.Contains(Type))
		{
			FCSKStatusVFXEntry& Entry = Effects.Add(Type);
			Entry.RelativeLocation = Location;
		}
	};

	AddDefault(ECSKStatusVisualType::Confusion, FVector(0.0f, 0.0f, 180.0f));
	AddDefault(ECSKStatusVisualType::Slow, FVector(0.0f, -100.0f, 70.0f));
	AddDefault(ECSKStatusVisualType::Haste, FVector(0.0f, 100.0f, 70.0f));
	AddDefault(ECSKStatusVisualType::DoughPickup, FVector(0.0f, 0.0f, 70.0f));

	// Constructor defaults must also initialize the class default object's entries.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		Effects[ECSKStatusVisualType::Confusion].RelativeLocation = FVector(0.0f, 0.0f, 180.0f);
		Effects[ECSKStatusVisualType::Slow].RelativeLocation = FVector(0.0f, -100.0f, 70.0f);
		Effects[ECSKStatusVisualType::Haste].RelativeLocation = FVector(0.0f, 100.0f, 70.0f);
		Effects[ECSKStatusVisualType::DoughPickup].RelativeLocation = FVector(0.0f, 0.0f, 70.0f);
	}
}
