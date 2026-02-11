#pragma once

#include "CoreMinimal.h"
#include "ACGameEnums.generated.h"

//마피아 탈출 임무 현황 열겨험
UENUM(BlueprintType)
enum class EEscapeState : uint8
{
	Contraband   UMETA(DisplayName = "Contraband"),
	BlackMarket   UMETA(DisplayName = "BlackMarket"),
    DeliverBomb UMETA(DisplayName = "DeliverBomb"),
    Escape      UMETA(DisplayName = "Escape"),
};

//입력매핑 컨텍스트 열거형
UENUM(BlueprintType)
enum class EInputMode : uint8
{
	Sholder		UMETA(DisplayName = "Sholder"),		// 기본
	Settings	UMETA(DisplayName = "Settings"),	// 설정창
	Spectator	UMETA(DisplayName = "Spectator"),	// 관전자
};

//설정창 열거형
UENUM(BlueprintType)
enum class ESettingMode : uint8
{
	None			UMETA(DisplayName = "Sholder"),		// 없음
	Default			UMETA(DisplayName = "Sholder"),		// 기본 설정창
	SteamFriendList	UMETA(DisplayName = "Settings"),	// 스팀 친구창
	Interact		UMETA(DisplayName = "Interact"),    // 인터랙트
	SoundSetting	UMETA(DisplayName = "SoundSetting"),// 사운드 설정
};

//플레이어 역할
UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
	None			UMETA(DisplayName = "None"),		// 없음
	Police			UMETA(DisplayName = "Police"),		// 경찰
	Mafia			UMETA(DisplayName = "Mafia"),		// 마피아
	Observer		UMETA(DisplayName = "Observer"),	// 관전자 (게임 진행 중 탈출을 해서 관전이 아닌 중간에 들어온 플레이어를 위한 타입)
};

//보이스 그룹
UENUM(BlueprintType)
enum class EVoiceGroup : uint8
{
	None	UMETA(DisplayName = "None"),	// None
	Lobby	UMETA(DisplayName = "Lobby"),	// Lobby
	Police	UMETA(DisplayName = "Police"),	// Police
	Mafia	UMETA(DisplayName = "Mafia"),	// Mafia
	Escape	UMETA(DisplayName = "Escape"),	// Escape
};

// 캐릭터 상태
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	None,
	Free,
	OnDamage,
	Interact,
	OnInteract,
	Stun,

	Angry,
	Prison,
	PrisonEscape,
	MAX_COUNT
};

// 캐릭터 위치 상태
UENUM(BlueprintType)
enum class ECharacterLocation : uint8
{
	None,
	Prison,
	Escape
};

// 캐릭터 타입 (상호작용 주체)
UENUM(BlueprintType)
enum class EACCharacterType : uint8
{
	None			UMETA(DisplayName = "None"),
	Police			UMETA(DisplayName = "Police"),
	Mafia			UMETA(DisplayName = "Mafia"),
	Citizen			UMETA(DisplayName = "Citizen"),
	BlackMarketDealer UMETA(DisplayName = "BlackMarketDealer"),
	Total
};

// 상호작용 대상 타입
UENUM(BlueprintType)
enum class EACInteractorType : uint8
{
	//None				UMETA(DisplayName = "None"),
	Police				UMETA(DisplayName = "Police"),
	Mafia				UMETA(DisplayName = "Mafia"),
	Citizen				UMETA(DisplayName = "Citizen"),				// 시민 NPC
	//Character			UMETA(DisplayName = "Character"),			// 다른 캐릭터 (플레이어 등)
	BlackMarketDealer	UMETA(DisplayName = "BlackMarketDealer"),	// 블랙마켓 딜러
	CCTVArea			UMETA(DisplayName = "CCTVArea"),			// CCTV
	ShopArea			UMETA(DisplayName = "ShopArea"),			// 상점
	PrisonDoor			UMETA(DisplayName = "PrisonDoor"),			// 감옥문
	Bomb				UMETA(DisplayName = "Bomb"),				// 탈출 임무 폭탄
	Contraband			UMETA(DisplayName = "Contraband"),				// 탈출 임무 밀수품
	//BombInstallArea		UMETA(DisplayName = "BombInstallArea"),		// 폭탄 설치 구역
	//EscapeArea			UMETA(DisplayName = "EscapeArea"),			// 탈출 구역
	Total
};

UENUM(BlueprintType)
enum class EInteractionKey : uint8
{
	E,	// 0
	R,	// 1
	T,	// 2
	Total
};

// 주요 시설 위치 정보
UENUM(BlueprintType)
enum class ESpot : uint8
{
	None				UMETA(DisplayName = "None"),
	Bank				UMETA(DisplayName = "Bank"),
};

// 게임 종료 유형
UENUM(BlueprintType)
enum class EGameEndType : uint8
{
	None		UMETA(DisplayName = "None"),
	Escape		UMETA(DisplayName = "Escape"),		// 탈출
	Score		UMETA(DisplayName = "Score"),		// 점수
	AllPrison	UMETA(DisplayName = "AllPrison"),	// 전원 감옥
	TimeOver	UMETA(DisplayName = "TimeOver"),	// 시간 종료
};

//알림 UI 종류
UENUM(BlueprintType)
enum class ENotificationType : uint8
{
	None		UMETA(DisplayName = "None"),
	Warning		UMETA(DisplayName = "Warning"),	//경고
};