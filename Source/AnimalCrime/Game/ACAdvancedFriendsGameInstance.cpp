
#include "ACAdvancedFriendsGameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/NetDriver.h"
#include "Engine/Engine.h"
#include "AnimalCrime.h"
#include "Components/AudioComponent.h"
#include "ACPlayerControllerBase.h"
#include "VoipListenerSynthComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/Common/ACFadeInScreen.h"
#include "AudioMixerBlueprintLibrary.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UACAdvancedFriendsGameInstance::UACAdvancedFriendsGameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// TransitionScreen 위젯 클래스 로드
	static ConstructorHelpers::FClassFinder<UACFadeInScreen> TransitionScreenRef(
		TEXT("/Game/Project/UI/Common/WBP_FadeIn.WBP_FadeIn_C")
	);
	if (TransitionScreenRef.Succeeded())
	{
		TransitionScreenClass = TransitionScreenRef.Class;
	}
}

#pragma region 엔진 제공 함수
void UACAdvancedFriendsGameInstance::Init()
{
	Super::Init();

	// 네트워크 끊김 처리
	GetEngine()->OnNetworkFailure().AddUObject(this, &UACAdvancedFriendsGameInstance::HandleNetworkFailure);
	GetEngine()->OnTravelFailure().AddUObject(this, &UACAdvancedFriendsGameInstance::HandleTravelFailure);

	OnWorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UACAdvancedFriendsGameInstance::OnWorldCleanup);

	// 맵 관련 초기화
	CurrentMapType = EMapType::None;

	MainMenuMapName = TEXT("MainMenuMap");
	LobbyMapName = TEXT("LobbyMap");
	GameMapName = TEXT("CityMap");

	// PIE(에디터 플레이) 환경에서도 SeamlessTravel 허용
	UE_LOG(LogTemp, Warning, TEXT("UACMainGameInstance::Init"));
	if (GEngine)
	{
		GEngine->Exec(GetWorld(), TEXT("net.AllowPIESeamlessTravel 1"));
	}

}

void UACAdvancedFriendsGameInstance::Shutdown()
{
	if (OnWorldCleanupHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(OnWorldCleanupHandle);
		OnWorldCleanupHandle.Reset();
	}
	Super::Shutdown();
}

void UACAdvancedFriendsGameInstance::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	UE_LOG(LogSY, Log, TEXT("=== OnWorldCleanup: %s ==="), *World->GetName());

	// VoipListenerSynthComponent 강제 정리
	for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
	{
		UVoipListenerSynthComponent* VoipComp = *It;
		if (IsValid(VoipComp))
		{
			UE_LOG(LogSY, Log, TEXT("OnWorldCleanup - Destroying VoipComp: %s"), *VoipComp->GetName());

			VoipComp->Stop();

			if (UAudioComponent* AudioComp = VoipComp->GetAudioComponent())
			{
				AudioComp->Stop();
				if (AudioComp->IsRegistered())
				{
					AudioComp->UnregisterComponent();
				}
				AudioComp->DestroyComponent();
			}

			if (VoipComp->IsRegistered())
			{
				VoipComp->UnregisterComponent();
			}
			VoipComp->DestroyComponent();
		}
	}

	UE_LOG(LogSY, Log, TEXT("=== OnWorldCleanup 완료 ==="));
}

void UACAdvancedFriendsGameInstance::OnStart()
{
	Super::OnStart();

	UE_LOG(LogSY, Warning, TEXT("UACMainGameInstance::OnStart"));
}
#pragma endregion

#pragma region Map Level
void UACAdvancedFriendsGameInstance::LoadMainMenuMap()
{
	UpdateMap(EMapType::MainMenu);
}
void UACAdvancedFriendsGameInstance::LoadLobbyMap()
{
	UpdateMap(EMapType::Lobby);
}

void UACAdvancedFriendsGameInstance::LoadGameMap()
{
	UpdateMap(EMapType::Game);
}

EMapType UACAdvancedFriendsGameInstance::GetMapType() const
{
	return CurrentMapType;
}

void UACAdvancedFriendsGameInstance::UpdateMap(const EMapType InMapType)
{
	if (CurrentMapType == InMapType)
	{
		UE_LOG(LogTemp, Error, TEXT("CurrentMapType == InMapType"));
		return;
	}

	CurrentMapType = InMapType;

	// Server Voice 인터페이스 정리 (패킷 중계 중지)
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		IOnlineVoicePtr Voice = Online::GetVoiceInterface();
		if (Voice.IsValid())
		{
			Voice->ClearVoicePackets();
			Voice->RemoveAllRemoteTalkers();
			Voice->StopNetworkedVoice(0);
			Voice->UnregisterLocalTalker(0);
		}
	}

	// Client들에게 Voice 정리 요청
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AACPlayerControllerBase* PC = Cast<AACPlayerControllerBase>(It->Get());
		if (PC != nullptr)
		{
			PC->Client_CleanupVoiceBeforeTravel();
		}
	}

	DoServerTravel();
}
#pragma endregion

void UACAdvancedFriendsGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogSY, Warning, TEXT("Network Failure: %s"), *ErrorString);

	// 세션 정리
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(World);
	if (SessionInterface.IsValid() == true)
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}
}

void UACAdvancedFriendsGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogSY, Warning, TEXT("Travel Failure: %s"), *ErrorString);

	// 세션 정리
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(World);
	if (SessionInterface.IsValid() == true)
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}
}

void UACAdvancedFriendsGameInstance::TryStartVoice()
{
	UE_LOG(LogSY, Log, TEXT("Try Start Voice"));

	IOnlineVoicePtr Voice = Online::GetVoiceInterface();
	if (Voice.IsValid() == false)
	{
		UE_LOG(LogSY, Log, TEXT("Voice: VoiceInterface 없음"));
		return;
	}

	IOnlineSessionPtr Session = Online::GetSessionInterface(GetWorld());
	if (Session.IsValid() == false || Session->GetNamedSession(NAME_GameSession) == nullptr)
	{
		UE_LOG(LogSY, Log, TEXT("Voice: 세션 없음, Voice 시작 안 함"));
		return;
	}

	// 보이스 연결 (중복 호출되어도 OnlineSubsystem이 처리)
	Voice->RegisterLocalTalker(0);
	Voice->StartNetworkedVoice(0);
	UE_LOG(LogSY, Log, TEXT("Voice initialized"));
}

void UACAdvancedFriendsGameInstance::DoServerTravel()
{
	UE_LOG(LogSY, Log, TEXT("=== DoServerTravel ==="));

	switch (CurrentMapType)
	{
	case EMapType::MainMenu:
		GetWorld()->ServerTravel("MainMenuMap", true);
		break;
	case EMapType::Lobby:
		GetWorld()->ServerTravel("LobbyMap", true);
		break;
	case EMapType::Game:
		GetWorld()->ServerTravel("CityMap", true);
		break;
	}
}

void UACAdvancedFriendsGameInstance::ShowTransitionScreen()
{
	if (TransitionScreenClass == nullptr)
	{
		UE_LOG(LogSY, Warning, TEXT("TransitionScreenClass is null"));
		return;
	}
	if (TransitionScreen == nullptr)
	{
		UE_LOG(LogSY, Log, TEXT("Creating TransitionScreen widget"));
		TransitionScreen = CreateWidget<UACFadeInScreen>(this, TransitionScreenClass);
	}

	if (TransitionScreen == nullptr)
	{
		UE_LOG(LogSY, Warning, TEXT("TransitionScreen is null"));
		return;
	}
	// GameViewportClient에 직접 추가 (핵심!) =====
	UGameViewportClient* Viewport = GetGameViewportClient();
	if (Viewport == nullptr)
	{
		UE_LOG(LogSY, Warning, TEXT("GameViewportClient is null"));
		return;
	}
	// 이미 추가되었는지 확인 (중복 방지)
	TSharedPtr<SWidget> WidgetSlate = TransitionScreen->TakeWidget();

	// ViewportWidgetContent로 추가 (맵 전환 시에도 유지됨)
	Viewport->AddViewportWidgetContent(WidgetSlate.ToSharedRef(), 1000);

	UE_LOG(LogSY, Log, TEXT("TransitionScreen added to GameViewportClient"));
}

void UACAdvancedFriendsGameInstance::HideTransitionScreen()
{
	if (TransitionScreen == nullptr)
	{
		return;
	}
	UGameViewportClient* Viewport = GetGameViewportClient();
	if (Viewport == nullptr)
	{
		return;
	}
	TSharedPtr<SWidget> WidgetSlate = TransitionScreen->TakeWidget();
	Viewport->RemoveViewportWidgetContent(WidgetSlate.ToSharedRef());

	UE_LOG(LogSY, Log, TEXT("TransitionScreen removed from GameViewportClient"));
}

void UACAdvancedFriendsGameInstance::ApplySavedAudioSettings()
{
	// 1. 출력 디바이스 적용
	if (SelectedAudioOutputDeviceId.IsEmpty() == false)
	{
		UE_LOG(LogSY, Log, TEXT("Applying saved audio output device: %s"), *SelectedAudioOutputDeviceId);

		FOnCompletedDeviceSwap OnSwapCompleted;
		OnSwapCompleted.BindUFunction(this, FName("OnAudioOutputDeviceSwapCompleted"));

		UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(
			this,
			SelectedAudioOutputDeviceId,
			OnSwapCompleted
		);
	}
	else
	{
		UE_LOG(LogSY, Log, TEXT("No saved audio output device"));
	}

#if PLATFORM_WINDOWS
	// 2. 입력 디바이스 적용 (Windows만)
	if (SelectedAudioInputDeviceId.IsEmpty() == true)
	{
		UE_LOG(LogSY, Log, TEXT("No saved audio input device"));
		return;
	}
	else
	{
		UE_LOG(LogSY, Log, TEXT("Validating saved input device: %s"), *SelectedAudioInputDeviceId);
	}

	//스팀 API에서 입력 디바이스 변경을 지원하지 않는 것으로 보임. 추후 수점
#endif
}

void UACAdvancedFriendsGameInstance::OnAudioOutputDeviceSwapCompleted(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Success)
	{
		UE_LOG(LogSY, Log, TEXT("[GameInstance] ✓ Audio output device applied successfully!"));
	}
	else
	{
		UE_LOG(LogSY, Warning, TEXT("[GameInstance] ✗ Failed to apply saved audio device"));
		// 실패하면 저장된 ID 초기화
		SelectedAudioOutputDeviceId.Empty();
	}
}

void UACAdvancedFriendsGameInstance::SetPlayerMicVolume(const FString& PlayerName, float Volume)
{
	PlayerMicVolumeMap.Add(PlayerName, FMath::Clamp(Volume, 0.f, 1.f));
}

float UACAdvancedFriendsGameInstance::GetPlayerMicVolume(const FString& PlayerName) const
{
	if (const float* Volume = PlayerMicVolumeMap.Find(PlayerName))
	{
		return *Volume;
	}
	return 0.5f; // 기본값
}

void UACAdvancedFriendsGameInstance::SetPlayerMicMute(const FString& PlayerName, bool bIsMute)
{
	PlayerMicMuteMap.Add(PlayerName, bIsMute);
}

bool UACAdvancedFriendsGameInstance::GetPlayerMicMute(const FString& PlayerName) const
{
	if (const bool* bIsMute = PlayerMicMuteMap.Find(PlayerName))
	{
		return *bIsMute;
	}
	return false; // 기본값
}