
#include "Character/ACLobbyCharacter.h"
#include "Game/ACLobbyPlayerController.h"
#include "Game/ACPlayerState.h"
#include "Game/ACLobbyGameState.h"
#include "EnhancedInputComponent.h"
#include "UI/GameStart/ACLobbyHeadInfo.h"
#include "Utils/ACBillboardComponent.h"
#include "Components/WidgetComponent.h"

#include "AnimalCrime.h"

AACLobbyCharacter::AACLobbyCharacter()
{
	HeadMesh->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Hat_057.SK_Hat_057")));

	Billboard = CreateDefaultSubobject<UACBillboardComponent>(TEXT("BillboardComponent"));
	Billboard->SetupAttachment(RootComponent);

	HeadInfo = CreateDefaultSubobject<UWidgetComponent>(TEXT("HeadInfo"));
	HeadInfo->SetupAttachment(Billboard);
	HeadInfo->SetWorldLocation(FVector(0.0f, 0.0f, 130.0f));
	HeadInfo->SetWidgetSpace(EWidgetSpace::World);
	HeadInfo->SetDrawSize(FVector2D(200.0f, 50.0f));
	HeadInfo->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FClassFinder<UACLobbyHeadInfo> HeadInfoRef(TEXT("/Game/Project/UI/GameStart/WBP_LobbyHeadInfo.WBP_LobbyHeadInfo_C"));
	if (HeadInfoRef.Succeeded())
	{
		HeadInfo->SetWidgetClass(HeadInfoRef.Class);
	}

	//로비에서는 라디오 모드
	VoiceGroup = EVoiceGroup::Lobby;
}

void AACLobbyCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHeadInfoName();

	UpdateHeadInfoIcon();
}

void AACLobbyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UpdateHeadInfoName();

	UpdateHeadInfoIcon();
}

void AACLobbyCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	UpdateHeadInfoName();

	UpdateHeadInfoIcon();
}

void AACLobbyCharacter::SetSteamFriendsList()
{
	AACLobbyPlayerController* PC = Cast<AACLobbyPlayerController>(GetController());
	if (PC == nullptr)
	{
		return;
	}

	//설정창이 꺼져있으면 스팀친구창 오픈, 스팀 친구창이 켜져있으면 끄기, 다른 설정창이면 아무것도 안함.
	if (SettingMode == ESettingMode::None)
	{
		PC->SteamFriendListToggle(true);
		ChangeInputMode(EInputMode::Settings);
		SettingMode = ESettingMode::SteamFriendList;
	}
	else if (SettingMode == ESettingMode::SteamFriendList)
	{
		PC->SteamFriendListToggle(false);
		ChangeInputMode(EInputMode::Sholder);
		SettingMode = ESettingMode::None;
	}
}

void AACLobbyCharacter::SettingsClose()
{
	switch (SettingMode)
	{
	case ESettingMode::None:
		break;
	case ESettingMode::Default:
		break;
	case ESettingMode::SoundSetting:
		SetSoundSetting();
		break;
	case ESettingMode::SteamFriendList:
		SetSteamFriendsList();
		break;
	default:
		break;
	}
}

void AACLobbyCharacter::UpdateHeadInfoName()
{
	AC_LOG(LogSY, Warning, TEXT("UpdateHeadInfoUI 호출됨"));
	if (HeadInfo == nullptr)
	{
		return;
	}

	UACLobbyHeadInfo* HeadInfoWidget = Cast<UACLobbyHeadInfo>(HeadInfo->GetWidget());
	if (HeadInfoWidget == nullptr)
	{
		return;
	}

	AACPlayerState* PS = GetPlayerState<AACPlayerState>();
	if (PS == nullptr)
	{
		return;
	}

	// 기본 PlayerState의 GetPlayerName() 사용
	FString PlayerName = PS->GetPlayerName();
	if (PlayerName.IsEmpty() == true)    
	{
		return;
	}
	AC_LOG(LogSY, Warning, TEXT("SetSteamNameText 호출 완료"));
	HeadInfoWidget->SetSteamNameText(PlayerName);
}

void AACLobbyCharacter::UpdateHeadInfoIcon()
{
	if (HeadInfo == nullptr)
	{
		return;
	}

	UACLobbyHeadInfo* HeadInfoWidget = Cast<UACLobbyHeadInfo>(HeadInfo->GetWidget());
	if (HeadInfoWidget == nullptr)
	{
		return;
	}

	AACPlayerState* PS = GetPlayerState<AACPlayerState>();
	if (PS == nullptr)
	{
		return;
	}

	AACLobbyGameState* GS = GetWorld()->GetGameState<AACLobbyGameState>();
	if (GS == nullptr)
	{
		return;
	}

	// 호스트 확인
	if (GS->GetHostPlayerState() == PS)
	{
		HeadInfoWidget->SetReadyIcon(ELobbyPlayerState::Host);
	}
	// Ready 상태 확인 (ReadyPlayerArray에 포함되어 있는지)
	else if (GS->IsPlayerReady(PS) == true)
	{
		HeadInfoWidget->SetReadyIcon(ELobbyPlayerState::Ready);
	}
	else
	{
		HeadInfoWidget->SetReadyIcon(ELobbyPlayerState::UnReady);
	}
}
