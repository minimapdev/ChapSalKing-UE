#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "Tteok/TteokData.h"
#include "CSKProjectSettingsData.generated.h"

class UDoughDataRegistry;
class UPestleItemData;
class UMortarZoneData;
class UCrownData;
class UCSKPlayerStatData;
class APestleItem;
class UDoughBlobData;
class ADoughBlob;
class UIngredientData;
class ALuckyPouch;
class UDoughSoftBodyData;
class UMaterialInterface;
class UNiagaraSystem;
class UCSKCameraShakeCatalog;
class USeesawData;
class UCSKStatusVFXCatalog;
class UCSKRootHUDWidget;
class UCSKMenuRootWidget;
class UCSKUIStyleData;
class UCSKIntroBookData;
class ACSKPlayerCharacter;
class ACSKBotAIController;

USTRUCT(BlueprintType)
struct FCSKPestleSpawnSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pestle|Spawn")
	TSubclassOf<APestleItem> PestleClass;

	// 웨이브 리드 타임. 매치 시작 직후·킹 종료 직후 시작되며, 끝나는 순간 복주머니 여러 개가
	// 동시에 착지하고 그중 하나에 절구공이가 들어 있다. 이 값이 곧 킹 등장 간격이다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pestle|Spawn", meta = (ClampMin = "0.01", Units = "s"))
	float SpawnCountdown = 30.0f;
};

USTRUCT(BlueprintType)
struct FCSKLobbySettings
{
	GENERATED_BODY()

	// 프리뷰 룸의 격자와 UI 슬롯 격자가 같은 값을 봐야 칸 수와 순서가 맞는다.
	// ACSKLobbyPreviewStage와 UCSKLobbyWidget이 함께 읽는 유일한 출처
	// (격자의 화면상 크기는 WBP_CSKLobby가 정한다 — 여기 있는 건 칸 개수뿐)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Grid", meta = (ClampMin = "1"))
	int32 Columns = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Grid", meta = (ClampMin = "1"))
	int32 Rows = 2;

	// 스탠드가 설 카메라 앞 거리. 반죽 크기는 칸의 화면 크기에서 역산하므로
	// 이 값은 원근의 세기만 정한다 — 가까울수록 가장자리 칸이 바깥으로 기운다.
	// 레벨 지오메트리를 뚫지 않을 만큼은 가까워야 한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Preview", meta = (ClampMin = "1.0", Units = "cm"))
	float PreviewDistance = 400.0f;

	// 반죽이 칸 세로를 채우는 비율. 1이면 칸 높이를 꽉 채운다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Preview", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float PreviewFillRatio = 0.8f;

	// 스탠드가 카메라를 보게 돌리는 각도. 정면(180)은 실루엣이 좌우대칭이라 납작해 보인다 —
	// 살짝 틀어 3/4 뷰로 두는 편이 입체감이 산다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Preview")
	float PreviewYaw = 205.0f;

	// 칸 바탕 카드. 슬롯 한 칸을 정확히 덮으며 반죽 뒤에 선다.
	// UMG는 언제나 3D 위에 그려져서 반죽 뒤에 아무것도 깔 수 없다 — 그래서 이것만 3D다.
	// 라이팅을 받으면 칸마다 밝기가 달라지므로 Unlit이어야 하고, 판을 좌우로 되뒤집어
	// 세우므로 Two Sided여야 한다(아니면 컬링돼 통째로 사라진다).
	// 비워두면 카드가 숨는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Preview")
	TSoftObjectPtr<UMaterialInterface> SlotCardMaterial;

	// 격자 뒤 전체를 덮어 레벨을 가리는 판. 화면을 정확히 채운다.
	// 역시 Unlit + Two Sided여야 한다.
	// 비워두면 판이 숨어 레벨이 그대로 배경이 된다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Preview")
	TSoftObjectPtr<UMaterialInterface> BackdropMaterial;

	// 카드가 반죽 뒤로 물러나는 여백. 반죽이 차지하는 반경만큼은 코드가 자동으로 더하므로
	// 여기엔 그 위에 얹을 여유만 준다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Preview", meta = (ClampMin = "0.0", Units = "cm"))
	float SlotCardDepthMargin = 20.0f;

	// 프리뷰 반죽의 대기 흔들림 설정. 인게임 폰이 쓰는 애셋과 별개의 애셋을 물려라 —
	// 로비는 가만히 선 반죽만 보여주는 자리라 흔들림을 크게 줘도 되고,
	// 여기서 무엇을 바꾸든 매치의 반죽에는 영향이 없다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Preview")
	TObjectPtr<UDoughSoftBodyData> PreviewSoftBodyData;

	// 배경 판이 반죽에서 뒤로 물러나는 거리. 카드보다 뒤여야 한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Preview", meta = (ClampMin = "1.0", Units = "cm"))
	float BackdropDistance = 500.0f;
};

USTRUCT(BlueprintType)
struct FCSKBotSpawnSettings
{
	GENERATED_BODY()

	// GameMode가 봇을 직접 스폰한다 — 레벨에 배치된 봇 캐릭터는 두지 않는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bot|Spawn")
	TSubclassOf<ACSKPlayerCharacter> BotPawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bot|Spawn")
	TSubclassOf<ACSKBotAIController> BotControllerClass;
};

USTRUCT(BlueprintType)
struct FCSKMatchSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match", meta = (ClampMin = "0.0"))
	float MatchDuration = 300.0f;

	// 프론트엔드에서 [시작]을 누르면 열리는 레벨
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
	TSoftObjectPtr<UWorld> MatchLevel;
};

USTRUCT(BlueprintType)
struct FCSKUISettings
{
	GENERATED_BODY()

	// ACSKHUD가 생성하는 인게임 HUD 루트. 미설정이면 HUD가 뜨지 않는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftClassPtr<UCSKRootHUDWidget> RootHUDWidgetClass;

	// ACSKMenuHUD가 생성하는 프론트엔드 루트. 미설정이면 타이틀 화면이 뜨지 않는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftClassPtr<UCSKMenuRootWidget> MenuRootWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCSKUIStyleData> UIStyleData;

	// 첫 실행에 타이틀보다 먼저 뜨는 동화책 인트로. 타이틀의 "다시 보기"도 같은 책을 연다.
	// 큰 그림 10장이라 소프트 참조 — 인트로 화면이 만들어질 때만 읽어 들인다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UCSKIntroBookData> IntroBookData;
};

USTRUCT(BlueprintType)
struct FCSKKingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "King", meta = (ClampMin = "0.0", Units = "s"))
	float KingSessionDuration = 30.0f;
};

// 복주머니에서 무엇이 나오는가. 절구공이는 여기 없다 — 확률로 뽑히면 등장이 보장되지 않으므로
// 웨이브마다 정확히 하나가 확정 배치된다 (FCSKPouchDropSettings::PestleWaveCount 참조)
UENUM(BlueprintType)
enum class ECSKPouchLootType : uint8
{
	Ingredient,
	Bomb
};

USTRUCT(BlueprintType)
struct FCSKPouchLootEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Loot")
	ECSKPouchLootType LootType = ECSKPouchLootType::Ingredient;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Loot",
		meta = (EditCondition = "LootType == ECSKPouchLootType::Ingredient"))
	TObjectPtr<UIngredientData> Ingredient;

	// 뽑기 가중치. 0이면 나오지 않는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Loot", meta = (ClampMin = "0"))
	int32 Weight = 1;
};

// 폭탄 주머니가 터질 때. 연 사람도 반경 안에 있으면 똑같이 맞는다 — 그래야
// "남이 먼저 열게 유도"가 성립한다
USTRUCT(BlueprintType)
struct FCSKPouchBombSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Bomb", meta = (ClampMin = "0.0", Units = "cm"))
	float Radius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Bomb", meta = (ClampMin = "0.0"))
	float KnockbackStrength = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Bomb", meta = (ClampMin = "0.0"))
	float KnockbackUpwardStrength = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Bomb", meta = (ClampMin = "0.0"))
	float TargetScaleResistancePower = 0.5f;

	// 잃는 몸집 비율. 손해가 크면 아무도 주머니를 안 줍게 되므로 작게 잡는다 — 넉백이 주고 G는 덤이다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Bomb", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropFraction = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Bomb")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Bomb", meta = (ClampMin = "0.01"))
	float ExplosionEffectScale = 1.0f;

	// CameraShake.Pouch.BombExplosion 세기. 셰이크 자산 자체는 카메라 셰이크 카탈로그가 정한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Bomb", meta = (ClampMin = "0.0"))
	float CameraShakeScale = 1.0f;
};

// 맵의 모든 보급이 이 설정 하나를 거친다. 지상 재료 스폰은 없다
USTRUCT(BlueprintType)
struct FCSKPouchDropSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Drop")
	TSubclassOf<ALuckyPouch> PouchClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Loot")
	TArray<FCSKPouchLootEntry> LootTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Bomb")
	FCSKPouchBombSettings BombSettings;

	// 상시 낙하 간격. 이 값이 "주머니 사이의 공백"을 결정한다 — 지상 재료가 없으므로
	// 길게 잡으면 그동안 할 일이 없어진다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Drop", meta = (ClampMin = "1.0", Units = "s"))
	float PouchInterval = 8.0f;

	// 절구공이 웨이브에서 동시에 떨어지는 개수. 그중 하나에만 절구공이가 들었다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Drop", meta = (ClampMin = "1"))
	int32 PestleWaveCount = 4;

	// 땅에 그림자 마커가 떠 있는 시간. 이게 있어야 미리 달려가는 선택이 생긴다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Drop", meta = (ClampMin = "0.0", Units = "s"))
	float ShadowLeadTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Drop", meta = (ClampMin = "0.1", Units = "s"))
	float FallDuration = 1.2f;

	// 낙하 시작 고도 (착지 지점 기준). FallDuration과 함께 체감 낙하 속도를 정한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Drop", meta = (ClampMin = "1.0", Units = "cm"))
	float DropHeight = 2000.0f;

	// 회수되지 않은 상시 주머니가 사라지는 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Drop", meta = (ClampMin = "1.0", Units = "s"))
	float PouchLifetime = 25.0f;

	// 절구공이 주머니가 스스로 터지는 시간. 아무도 안 열면 매치 루프가 멈추므로 반드시 필요하다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Drop", meta = (ClampMin = "1.0", Units = "s"))
	float PestlePouchAutoOpenDelay = 15.0f;

	// 이 고도를 넘는 지점에는 떨어지지 않는다 — 고지대는 안전하되 무자원으로 남긴다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Location", meta = (Units = "cm"))
	float MaxDropZ = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Location", meta = (ClampMin = "0.0", Units = "cm"))
	float MinDropSpacing = 800.0f;

	// 웨이브끼리 떨어져야 하는 거리. 흩어지게 하는 것이 목적이므로 MinDropSpacing보다 훨씬 크다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Location", meta = (ClampMin = "0.0", Units = "cm"))
	float MinWaveSpacing = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|Location", meta = (ClampMin = "0.0", Units = "cm"))
	float DropClearanceRadius = 100.0f;

	// 그림자 마커 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|VFX")
	TObjectPtr<UNiagaraSystem> ShadowEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|VFX", meta = (ClampMin = "0.01"))
	float ShadowEffectScale = 1.0f;

	// 땅에 닿는 순간. 물리를 쓰지 않으므로 "떨어졌다"는 느낌은 전적으로 이 연출이 만든다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|VFX")
	TObjectPtr<UNiagaraSystem> LandingEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|VFX", meta = (ClampMin = "0.01"))
	float LandingEffectScale = 1.0f;

	// CameraShake.Pouch.Landing 세기. 0이면 흔들지 않는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|VFX", meta = (ClampMin = "0.0"))
	float LandingCameraShakeScale = 1.0f;

	// 주머니가 터질 때
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|VFX")
	TObjectPtr<UNiagaraSystem> OpenEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pouch|VFX", meta = (ClampMin = "0.01"))
	float OpenEffectScale = 1.0f;
};

/**
 * 찹쌀킹 프로젝트 전역 데이터 허브.
 */
UCLASS(BlueprintType)
class CHAPSALKING_API UCSKProjectSettingsData : public UCSKDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dough")
	TObjectPtr<UDoughDataRegistry> DoughDataRegistry;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Pestle")
	TObjectPtr<UPestleItemData> PestleItemData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Mortar")
	TObjectPtr<UMortarZoneData> MortarZoneData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Seesaw")
	TObjectPtr<USeesawData> SeesawData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Pestle")
	FCSKPestleSpawnSettings PestleSpawnSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
	FCSKMatchSettings MatchSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bot")
	FCSKBotSpawnSettings BotSpawnSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	FCSKLobbySettings LobbySettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "King")
	FCSKKingSettings KingSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|LuckyPouch")
	FCSKPouchDropSettings PouchDropSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|DoughBlob")
	TSubclassOf<ADoughBlob> DoughBlobClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|DoughBlob")
	TObjectPtr<UDoughBlobData> DoughBlobData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Crown")
	TObjectPtr<UCrownData> CrownData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
	TObjectPtr<UCSKPlayerStatData> PlayerStatData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Status VFX")
	TObjectPtr<UCSKStatusVFXCatalog> StatusVFXCatalog;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCSKCameraShakeCatalog> CameraShakeCatalog;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tteok")
	TMap<ETteokType, TObjectPtr<UTteokData>> TteokDataMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FCSKUISettings UISettings;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};

