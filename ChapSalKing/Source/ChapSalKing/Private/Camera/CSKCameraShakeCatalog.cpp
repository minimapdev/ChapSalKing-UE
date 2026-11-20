#include "Camera/CSKCameraShakeCatalog.h"

const FCSKCameraShakeEntry* UCSKCameraShakeCatalog::FindEntry(const FGameplayTag& ShakeTag) const
{
	return Entries.FindByPredicate(
		[&ShakeTag](const FCSKCameraShakeEntry& Entry)
		{
			return Entry.ShakeTag == ShakeTag;
		});
}
