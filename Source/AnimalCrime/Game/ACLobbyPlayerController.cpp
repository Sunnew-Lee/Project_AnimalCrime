
#include "ACLobbyPlayerController.h"
#include "UI/GameStart/ACLobbyScreen.h"
#include "UI/Common/ACFadeInScreen.h"
#include "Blueprint/UserWidget.h"
#include "ACLobbyGameMode.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "InputActionValue.h"
#include "Character/ACLobbyCharacter.h"
#include "ACLobbyGameState.h"
#include "AnimalCrime.h"
#include "GameFramework/PlayerState.h"
#include "ACAdvancedFriendsGameInstance.h"

AACLobbyPlayerController::AACLobbyPlayerController()
{
	//Lobby 스크린 로드
	static ConstructorHelpers::FClassFinder<UACLobbyScreen> LobbyScreenRef(TEXT("/Game/Project/UI/GameStart/WBP_LobbyScreen.WBP_LobbyScreen_C"));
	if (LobbyScreenRef.Succeeded())
	{
		LobbyScreenClass = LobbyScreenRef.Class;
	}

	//SteamFriendList 로드
	static ConstructorHelpers::FClassFinder<UUserWidget> SteamFriendListRef(TEXT("/Game/Project/UI/GameStart/WBP_FriendList.WBP_FriendList_C"));
	if (SteamFriendListRef.Succeeded())
	{
		SteamFriendListClass = SteamFriendListRef.Class;
	}

	// ===== 입력 관련 로드 =====
	static ConstructorHelpers::FObjectFinder<UInputAction> SteamFriendListActionRef(TEXT("/Game/Project/Input/Actions/IA_SteamFriendList.IA_SteamFriendList"));
	if (SteamFriendListActionRef.Succeeded())
	{
		SteamFriendListAction = SteamFriendListActionRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ReadyActionRef(TEXT("/Game/Project/Input/Actions/IA_Ready.IA_Ready"));
	if (ReadyActionRef.Succeeded())
	{
		ReadyAction = ReadyActionRef.Object;
	}
}

void AACLobbyPlayerController::SteamFriendListToggle(bool bVisible)
{
	if (SteamFriendList == nullptr)
	{
		AC_LOG(LogSY, Warning, TEXT("SteamFriendList is nullptr"));
		return;
	}

	if (bVisible == true)
	{
		AC_LOG(LogSY, Log, TEXT("open"));
		SteamFriendList->AddToViewport();

		// 마우스 커서 켜기
		bShowMouseCursor = true;

		// UI와 게임 입력 둘 다 받기
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		AC_LOG(LogSY, Log, TEXT("close"));
		SteamFriendList->RemoveFromParent();
		// 마우스 커서 끄기
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
		SetInputMode(FInputModeGameOnly());
	}

}

void AACLobbyPlayerController::UpdatePlayerReadyText(int32 AllplayerNum)
{
	LobbyScreen->UpdatePlayerReadyText(0, AllplayerNum);
}

void AACLobbyPlayerController::ServerStartGame_Implementation()
{
	AGameModeBase* GMBase = GetWorld()->GetAuthGameMode();
	if (GMBase == nullptr)
	{
		return;
	}

	AACLobbyGameMode* GM = Cast< AACLobbyGameMode>(GMBase);
	if (GM == nullptr)
	{
		return;
	}

	GM->StartGamePlay();

}

void AACLobbyPlayerController::ServerReadyToggle_Implementation()
{
	bIsReady = !bIsReady;

	AACLobbyGameState* GS = GetWorld()->GetGameState<AACLobbyGameState>();
	if (GS == nullptr)
	{
		return;
	}

	GS->SetReadyPlayer(GetPlayerState<APlayerState>(), bIsReady);

}

void AACLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어만 UI 생성, 보이스 연결
	if (IsLocalController() == false)
	{
		return;
	}

	//UI 연동
	LobbyScreen = CreateWidget<UACLobbyScreen>(this, LobbyScreenClass);
	if (LobbyScreen == nullptr)
	{
		return;
	}
	LobbyScreen->AddToViewport();

	//// 마우스 커서 숨김
	//bShowMouseCursor = false;

	//// 클릭 끔
	//bEnableClickEvents = false;
	//bEnableMouseOverEvents = false;

	//// 게임 입력만 받기
	//SetInputMode(FInputModeGameOnly());

	// SteamFriendList 생성
	SteamFriendList = CreateWidget<UUserWidget>(this, SteamFriendListClass);
}

void AACLobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		AC_LOG(LogTemp, Error, TEXT("EnhancedInputComponent is nullptr"));
		return;
	}

	// 입력 액션 바인딩
	if (SteamFriendListAction)
	{
		EnhancedInputComponent->BindAction(SteamFriendListAction, ETriggerEvent::Started, this, &AACLobbyPlayerController::HandleSteamFriendList);
	}
	if (ReadyAction)
	{
		EnhancedInputComponent->BindAction(ReadyAction, ETriggerEvent::Started, this, &AACLobbyPlayerController::HandleGameReady);
	}
}

// ===== 입력 핸들러 구현 =====

void AACLobbyPlayerController::HandleSteamFriendList(const struct FInputActionValue& Value)
{
	AACLobbyCharacter* LobbyChar = GetPawn<AACLobbyCharacter>();
	if (LobbyChar == nullptr)
	{
		return;
	}

	LobbyChar->SetSteamFriendsList();
}

void AACLobbyPlayerController::HandleGameReady(const struct FInputActionValue& Value)
{

	// 호스트인지 확인
	if (IsLocalController() && HasAuthority())
	{
		//모든 인원 준비 했는지 확인
		AACLobbyGameState* GS = GetWorld() ? GetWorld()->GetGameState<AACLobbyGameState>() : nullptr;
		if (GS == nullptr)
		{
			return;
		}
		if (GS->GetReadyPlayerCount() != GS->GetAllPlayerCount())
		{
			AC_LOG(LogSY, Log, TEXT("호스트가 게임 시작을 눌렀으나 모든 플레이어가 준비되지 못함"));
			return;
		}

		AC_LOG(LogSY, Log, TEXT("호스트 - 게임 시작"));

		//모든 플레이어에게 FadeIn 스크린 붙이기
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AACLobbyPlayerController* PC = Cast<AACLobbyPlayerController>(It->Get());
			if (PC)
			{
				PC->ClientPlayFadeIn();
			}
		}

	}
	else
	{
		AC_LOG(LogSY, Log, TEXT("클라이언트 - Ready 토글"));
		bIsReady = !bIsReady;
		ServerReadyToggle();
	}
}

void AACLobbyPlayerController::ClientPlayFadeIn_Implementation()
{
	UACAdvancedFriendsGameInstance* GI = Cast<UACAdvancedFriendsGameInstance>(GetGameInstance());
	if (GI == nullptr)
	{
		return;
	}
	GI->ShowTransitionScreen(); // 먼저 화면 가리기
	if (GI->TransitionScreen == nullptr)
	{
		return;
	}

	if (HasAuthority())
	{
		GI->TransitionScreen->OnFadeInFinished.AddDynamic(this, &AACLobbyPlayerController::OnGameStartFadeInFinished);
	}
}

void AACLobbyPlayerController::OnGameStartFadeInFinished()
{
	ServerStartGame();
}