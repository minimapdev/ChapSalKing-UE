#include "AI/CSKBotAIConfigData.h"

FCSKBotPersonality UCSKBotAIConfigData::RollPersonality() const
{
	FCSKBotPersonality P;
	P.Aggression = AggressionRange.Roll();
	P.Greed = GreedRange.Roll();
	P.Ambition = AmbitionRange.Roll();
	P.Accuracy = AccuracyRange.Roll();
	P.Reflex = ReflexRange.Roll();
	P.TimingScale = TimingScaleRange.Roll();
	P.DashScale = DashScaleRange.Roll();
	P.JumpScale = JumpScaleRange.Roll();
	P.StruggleFlipsPerSecond = StruggleFlipsPerSecondRange.Roll();

	const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
	const float Radius = SurroundRadiusRange.Roll();
	P.SurroundOffset = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

	return P;
}
