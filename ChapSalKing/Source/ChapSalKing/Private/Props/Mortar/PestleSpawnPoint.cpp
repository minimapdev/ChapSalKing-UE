#include "Props/Mortar/PestleSpawnPoint.h"
#include "Components/SceneComponent.h"

#if WITH_EDITORONLY_DATA
#include "Components/ArrowComponent.h"
#endif

APestleSpawnPoint::APestleSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

#if WITH_EDITORONLY_DATA
	EditorArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("EditorArrow"));
	if (EditorArrow)
	{
		EditorArrow->SetupAttachment(SceneRoot);
		EditorArrow->SetArrowColor(FColor::Yellow);
		EditorArrow->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
		EditorArrow->bIsEditorOnly = true;
	}
#endif
}
