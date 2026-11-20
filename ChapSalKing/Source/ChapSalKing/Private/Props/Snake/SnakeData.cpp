#include "Props/Snake/SnakeData.h"

USnakeData::USnakeData()
{
	// 사양 3.2의 표를 그대로 옮긴다. 에디터에서 에셋을 만들면 이 값으로 시작한다
	FSnakeModeSettings& Orbit = Modes.AddDefaulted_GetRef();
	Orbit.Mode = ESnakeMode::Orbit;
	Orbit.SpeedMultiplier = 1.0f;
	Orbit.SelectionWeight = 40.0f;
	Orbit.MinDuration = 10.0f;
	Orbit.MaxDuration = 20.0f;
	Orbit.TelegraphDuration = 0.0f;

	FSnakeModeSettings& Cross = Modes.AddDefaulted_GetRef();
	Cross.Mode = ESnakeMode::Cross;
	Cross.SpeedMultiplier = 1.6f;
	Cross.SelectionWeight = 25.0f;
	Cross.MinDuration = 0.0f;
	Cross.MaxDuration = 0.0f;
	Cross.TelegraphDuration = 1.5f;

	FSnakeModeSettings& Coil = Modes.AddDefaulted_GetRef();
	Coil.Mode = ESnakeMode::Coil;
	Coil.SpeedMultiplier = 0.9f;
	Coil.SelectionWeight = 20.0f;
	Coil.MinDuration = 0.0f;
	Coil.MaxDuration = 0.0f;
	Coil.TelegraphDuration = 0.0f;

	FSnakeModeSettings& Lurk = Modes.AddDefaulted_GetRef();
	Lurk.Mode = ESnakeMode::Lurk;
	Lurk.SpeedMultiplier = 1.3f;
	Lurk.SelectionWeight = 15.0f;
	Lurk.MinDuration = 4.0f;
	Lurk.MaxDuration = 8.0f;
	Lurk.TelegraphDuration = 1.5f;
}
