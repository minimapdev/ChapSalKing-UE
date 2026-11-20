#include "UI/CSKUIStyleData.h"

FLinearColor UCSKUIStyleData::GetAffinityColor(ECSKAffinity Affinity) const
{
	if (const FLinearColor* Found = AffinityColors.Find(Affinity))
	{
		return *Found;
	}
	return NeutralColor;
}

FLinearColor UCSKUIStyleData::GetEventFeedColor(ECSKMatchEventType EventType) const
{
	if (const FLinearColor* Found = EventFeedColors.Find(EventType))
	{
		return *Found;
	}
	return NeutralColor;
}
