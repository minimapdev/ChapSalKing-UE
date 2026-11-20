#include "Player/Components/CSKPlayerVisualComponent.h"

#include "Dough/DoughData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"
#include "Ingredient/IngredientData.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

static const FName BodySlotName(TEXT("Body"));

UCSKPlayerVisualComponent::UCSKPlayerVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCSKPlayerVisualComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACSKPlayerCharacter* Character = Cast<ACSKPlayerCharacter>(GetOwner()))
	{
		Character->GetIngredientComponent()->OnIngredientStateChanged.AddDynamic(
			this,
			&UCSKPlayerVisualComponent::HandleIngredientStateChanged
		);
	}
}

void UCSKPlayerVisualComponent::HandleIngredientStateChanged(UIngredientData* Ingredient, float GaugeRatio)
{
	if (bTteokOverride)
	{
		return;
	}

	SetBodyOverlay(Ingredient ? Ingredient->BodyOverlayMaterial : nullptr);

	if (BodyOverlayMID)
	{
		BodyOverlayMID->SetScalarParameterValue(TEXT("Fade"), GaugeRatio);
	}
}

void UCSKPlayerVisualComponent::SetTteokBodyOverlay(UMaterialInterface* Material)
{
	bTteokOverride = (Material != nullptr);

	if (bTteokOverride)
	{
		SetBodyOverlay(Material);
		return;
	}

	const ACSKPlayerCharacter* Character = Cast<ACSKPlayerCharacter>(GetOwner());
	if (const UCSKPlayerIngredientComponent* IngredientComp = Character ? Character->GetIngredientComponent() : nullptr)
	{
		HandleIngredientStateChanged(IngredientComp->GetCurrentIngredient(), IngredientComp->GetGaugeRatio());
	}
	else
	{
		SetBodyOverlay(nullptr);
	}
}

void UCSKPlayerVisualComponent::SetBodyOverlay(UMaterialInterface* Material)
{
	if (Material == AppliedBodyOverlay)
	{
		return;
	}

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	USkeletalMeshComponent* MeshComp = Character ? Character->GetMesh() : nullptr;
	if (!MeshComp)
	{
		return;
	}

	const int32 BodyIndex = MeshComp->GetMaterialIndex(BodySlotName);
	if (BodyIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] SetBodyOverlay: Body 머티리얼 슬롯이 없음"));
		return;
	}

	BodyOverlayMID = nullptr;
	AppliedBodyOverlay = nullptr;

	TArray<TObjectPtr<UMaterialInterface>>& SlotOverlays = MeshComp->MaterialSlotsOverlayMaterial;
	if (Material)
	{
		BodyOverlayMID = UMaterialInstanceDynamic::Create(Material, this);
		AppliedBodyOverlay = Material;

		if (SlotOverlays.Num() <= BodyIndex)
		{
			SlotOverlays.SetNum(BodyIndex + 1);
		}
		SlotOverlays[BodyIndex] = BodyOverlayMID;
	}
	else if (SlotOverlays.IsValidIndex(BodyIndex))
	{
		SlotOverlays[BodyIndex] = nullptr;
	}

	MeshComp->PrecachePSOs();
	MeshComp->MarkRenderStateDirty();
}

void UCSKPlayerVisualComponent::ApplyVisualData(UDoughData* DoughData)
{
	if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		ApplyDoughVisualToMesh(DoughData, OwnerCharacter->GetMesh());
	}
}

void UCSKPlayerVisualComponent::ApplyDoughVisualToMesh(UDoughData* DoughData, USkeletalMeshComponent* MeshComp)
{
	if (!DoughData || !MeshComp)
	{
		return;
	}

	auto CreateMID = [MeshComp](FName SlotName) -> UMaterialInstanceDynamic*
	{
		const int32 MaterialIndex = MeshComp->GetMaterialIndex(SlotName);
		if (MaterialIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("머티리얼 슬롯을 찾을 수 없습니다: %s"), *SlotName.ToString());
			return nullptr;
		}
		return MeshComp->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
	};

	if (UMaterialInstanceDynamic* BodyMID = CreateMID(BodySlotName))
	{
		BodyMID->SetVectorParameterValue(TEXT("BodyColor"), DoughData->BodyColor);
	}

	if (UMaterialInstanceDynamic* CheekLMID = CreateMID(TEXT("Cheek_L")))
	{
		if (DoughData->CheekTexture)
		{
			CheekLMID->SetTextureParameterValue(TEXT("FaceTexture"), DoughData->CheekTexture);
		}
	}

	if (UMaterialInstanceDynamic* CheekRMID = CreateMID(TEXT("Cheek_R")))
	{
		if (DoughData->CheekTexture)
		{
			CheekRMID->SetTextureParameterValue(TEXT("FaceTexture"), DoughData->CheekTexture);
		}
	}

	if (UMaterialInstanceDynamic* EyeLMID = CreateMID(TEXT("Eye_L")))
	{
		if (DoughData->EyeLTexture)
		{
			EyeLMID->SetTextureParameterValue(TEXT("FaceTexture"), DoughData->EyeLTexture);
		}
	}

	if (UMaterialInstanceDynamic* EyeRMID = CreateMID(TEXT("Eye_R")))
	{
		if (DoughData->EyeRTexture)
		{
			EyeRMID->SetTextureParameterValue(TEXT("FaceTexture"), DoughData->EyeRTexture);
		}
	}

	if (UMaterialInstanceDynamic* MouthMID = CreateMID(TEXT("Mouth")))
	{
		if (DoughData->MouthTexture)
		{
			MouthMID->SetTextureParameterValue(TEXT("FaceTexture"), DoughData->MouthTexture);
		}
		MouthMID->SetVectorParameterValue(TEXT("FaceColor"), DoughData->MouthTintColor);
	}
}
