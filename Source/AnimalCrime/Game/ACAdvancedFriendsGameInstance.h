
#pragma once

#include "CoreMinimal.h"
#include "ACGameEnums.h"
#include "AdvancedFriendsGameInstance.h"
#include "ACAdvancedFriendsGameInstance.generated.h"

UENUM(BlueprintType)
enum class EMapType : uint8
{
	None		UMETA(DisplayName = "None"),
	MainMenu	UMETA(DisplayName = "MainMenu"),
	Lobby       UMETA(DisplayName = "Lobby"),
	Game        UMETA(DisplayName = "Game"),
	Result      UMETA(DisplayName = "Result"),
	Shop	    UMETA(DisplayName = "Shop"),
	Unknown     UMETA(DisplayName = "Unknown")
};

UCLASS()
class ANIMALCRIME_API UACAdvancedFriendsGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public:
	UACAdvancedFriendsGameInstance(const FObjectInitializer& ObjectInitializer);
#pragma region 엔진 제공 함수
public:
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnStart() override;
#pragma endregion

#pragma region Map Level 
public:
	void LoadMainMenuMap();
	void LoadLobbyMap();
	void LoadGameMap();

	EMapType GetMapType() const;

private:
	void UpdateMap(EMapType InMapType);

	//void BeginLoadingScreen(const FString& MapName);
	//void OnSeamlessTravelStart(UWorld* CurrentWorld, const FString& LevelName);
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

#pragma endregion

public:
	/** 실제 세션 생성은 BP에서 구현 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Session")
	void CreateSession();

protected:

	/**
		@brief 네트워크 실패 시 처리 로직
		@param World       - 어느 월드에서 발생했는지
		@param NetDriver   - 어떤 네트워크 드라이버인지
		@param FailureType - 실패 타입 (연결 끊김, 타임아웃 등)
		@param ErrorString - 에러 메시지
	**/
	UFUNCTION()
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	/**
		@brief 맵 이동 실패 시 처리 로직
		@param World       - 어느 월드에서 발생했는지
		@param FailureType - 실패 타입
		@param ErrorString - 에러 메시지
	**/
	UFUNCTION()
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

public:
	/**
		@brief 음성 채팅(보이스)을 시작하기 위한 초기화 시도 함수
	**/
	void TryStartVoice();

private:
	void DoServerTravel();

public:
	/**
		@brief 맵 이동 중 대채 화면 표시
	**/
	UFUNCTION(BlueprintCallable)
	void ShowTransitionScreen();

	/**
		@brief 대체 화면 숨기기
	**/
	UFUNCTION(BlueprintCallable)
	void HideTransitionScreen();

#pragma region Map Level 관련 맴버 변수 
private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	EMapType CurrentMapType;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	FString MainMenuMapName;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	FString LobbyMapName;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	FString GameMapName;

	FDelegateHandle OnWorldCleanupHandle;
#pragma endregion

public:
	//!< 플레이어 역할 저장
	TMap<FUniqueNetIdRepl, EPlayerRole> SavedPlayerRoles;

protected:
	//!< 맵 전환 시 화면을 가리는 영구 위젯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UACFadeInScreen> TransitionScreenClass;
public:
	UPROPERTY()
	TObjectPtr<class UACFadeInScreen> TransitionScreen;

public:
	//!< 오디오 디바이스 ID
	UPROPERTY()
	FString SelectedAudioOutputDeviceId;
};

