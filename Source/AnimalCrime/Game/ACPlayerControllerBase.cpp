
#include "ACPlayerControllerBase.h"
#include "ACAdvancedFriendsGameInstance.h"
#include "AudioDevice.h"
#include "InputActionValue.h"
#include "Character/ACCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "AnimalCrime.h"
#include "UI/Sound/ACSoundSetting.h"

AACPlayerControllerBase::AACPlayerControllerBase()
{
	//SoundSetting 로드
	static ConstructorHelpers::FClassFinder<UACSoundSetting> SoundSettingRef(TEXT("/Game/Project/UI/Sound/WBP_SoundSetting.WBP_SoundSetting_C"));
	if (SoundSettingRef.Succeeded())
	{
		SoundSettingWidgetClass = SoundSettingRef.Class;
	}

	// ===== 공통 입력 관련 로드 =====
	static ConstructorHelpers::FObjectFinder<UInputMappingContext>
		DefaultMappingContextRef(TEXT("/Game/Project/Input/IMC_Shoulder.IMC_Shoulder"));
	if (DefaultMappingContextRef.Succeeded())
	{
		DefaultMappingContext = DefaultMappingContextRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>
		MoveActionRef(TEXT("/Game/Project/Input/Actions/IA_Move.IA_Move"));
	if (MoveActionRef.Succeeded())
	{
		MoveAction = MoveActionRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>
		LookActionRef(TEXT("/Game/Project/Input/Actions/IA_Look.IA_Look"));
	if (LookActionRef.Succeeded())
	{
		LookAction = LookActionRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>
		JumpActionRef(TEXT("/Game/Project/Input/Actions/IA_Jump.IA_Jump"));
	if (JumpActionRef.Succeeded())
	{
		JumpAction = JumpActionRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>
		InteractActionRef(TEXT("/Game/Project/Input/Actions/IA_Interact.IA_Interact"));
	if (InteractActionRef.Succeeded())
	{
		InteractAction = InteractActionRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>
		ItemDropActionRef(TEXT("/Game/Project/Input/Actions/IA_ItemDrop.IA_ItemDrop"));
	if (ItemDropActionRef.Succeeded())
	{
		ItemDropAction = ItemDropActionRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>
		MeleeActionRef(TEXT("/Game/Project/Input/Actions/IA_Attack.IA_Attack"));
	if (MeleeActionRef.Succeeded())
	{
		MeleeAction = MeleeActionRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> SprintActionRef(TEXT("/Game/Project/Input/Actions/IA_Sprint.IA_Sprint"));
	if (SprintActionRef.Succeeded())
	{
		SprintAction = SprintActionRef.Object;
	}

	// ===== 설정창 입력 로드 =====
	static ConstructorHelpers::FObjectFinder<UInputMappingContext>
		SettingsMappingContextRef(TEXT("/Game/Project/Input/IMC_Settings.IMC_Settings"));
	if (SettingsMappingContextRef.Succeeded())
	{
		SettingsMappingContext = SettingsMappingContextRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>
		SettingsCloseActionRef(TEXT("/Game/Project/Input/Actions/IA_SettingsClose.IA_SettingsClose"));
	if (SettingsCloseActionRef.Succeeded())
	{
		SettingsCloseAction = SettingsCloseActionRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>
		SoundSettingActionRef(TEXT("/Game/Project/Input/Actions/IA_SoundSetting.IA_SoundSetting"));
	if (SoundSettingActionRef.Succeeded())
	{
		SoundSettingAction = SoundSettingActionRef.Object;
	}
}

void AACPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	// 오디오 설정 적용
	GetWorldTimerManager().SetTimer(
		AudioSettingsTimer,
		this,
		&AACPlayerControllerBase::ApplyAudioSettings,
		0.5f,
		false
	);

	//UI 연동

	// 마우스 커서 숨김
	bShowMouseCursor = false;

	// 클릭 끔
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;

	// 게임 입력만 받기
	SetInputMode(FInputModeGameOnly());

	// SteamFriendList 생성
	SoundSettingWidget = CreateWidget<UACSoundSetting>(this, SoundSettingWidgetClass);
	SoundSettingWidget->SetVisibility(ESlateVisibility::Hidden);
	SoundSettingWidget->AddToViewport(1000);
}

void AACPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("EnhancedInputComponent is nullptr"));
		return;
	}

	// Enhanced Input Subsystem에 기본 매핑 컨텍스트 추가
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// 공통 입력 액션 바인딩
	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AACPlayerControllerBase::HandleMove);
	}
	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AACPlayerControllerBase::HandleLook);
	}
	if (JumpAction)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AACPlayerControllerBase::HandleJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AACPlayerControllerBase::HandleStopJumping);
	}
	if (InteractAction)
	{
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AACPlayerControllerBase::HandleInteractStart);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AACPlayerControllerBase::HandleInteractHold);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AACPlayerControllerBase::HandleInteractRelease);
	}
	if (ItemDropAction)
	{
		EnhancedInputComponent->BindAction(ItemDropAction, ETriggerEvent::Started, this, &AACPlayerControllerBase::HandleItemDrop);
	}
	if (MeleeAction)
	{
		EnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Started, this, &AACPlayerControllerBase::HandleAttack);
	}
	if (SettingsCloseAction)
	{
		EnhancedInputComponent->BindAction(SettingsCloseAction, ETriggerEvent::Started, this, &AACPlayerControllerBase::HandleSettingsClose);
	}
	if (SoundSettingAction)
	{
		EnhancedInputComponent->BindAction(SoundSettingAction, ETriggerEvent::Started, this, &AACPlayerControllerBase::HandleSoundSetting);
	}
	// 캐릭터 스킬 - Sprint
	if (SprintAction)
	{
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AACPlayerControllerBase::HandleSprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AACPlayerControllerBase::HandleSprintEnd);
		// EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AACMainPlayerController::HandleSprintEnd);
	}
}

void AACPlayerControllerBase::ApplyAudioSettings()
{
	UACAdvancedFriendsGameInstance* GI = GetGameInstance<UACAdvancedFriendsGameInstance>();
	if (GI == nullptr)
	{
		return;
	}
	GI->ApplySavedAudioSettings();
}

void AACPlayerControllerBase::HandleMove(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	ControlledCharacter->Move(Value);
}

void AACPlayerControllerBase::HandleLook(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	ControlledCharacter->Look(Value);
}

void AACPlayerControllerBase::HandleJump(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	ControlledCharacter->Jump();
}

void AACPlayerControllerBase::HandleStopJumping(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	ControlledCharacter->StopJumping();
}

void AACPlayerControllerBase::HandleInteractStart(const FInputActionValue& Value)
{
	// 1. float 값 추출 (E=1.0, R=2.0, T=3.0)
	float InputFloat = Value.Get<float>();
	//UE_LOG(LogSW, Log, TEXT("Interact [%f] Key Input!!"), InputFloat);

	// 2. 정수 변환
	int32 InputIndex = FMath::RoundToInt(InputFloat);
	//UE_LOG(LogSW, Log, TEXT("Interact [%d] Key Input!!"), InputIndex);

	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
		return;

	// 3. 인덱스 전달
	ControlledCharacter->InteractStarted(InputIndex - 1);
}

void AACPlayerControllerBase::HandleInteractHold(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	ControlledCharacter->InteractHolding(GetWorld()->GetDeltaSeconds());
}

void AACPlayerControllerBase::HandleInteractRelease(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	ControlledCharacter->InteractReleased();
}

void AACPlayerControllerBase::HandleItemDrop(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	ControlledCharacter->ItemDrop();
}

void AACPlayerControllerBase::HandleAttack(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	ControlledCharacter->Attack();
}

void AACPlayerControllerBase::HandleSprintStart(const struct FInputActionValue& Value)
{
	if (CanUseSkill() == false)
	{
		AC_LOG(LogHY, Error, TEXT("CanUseSkill is false"));
		return;
	}

	AACCharacter* CharacterPawn = GetPawn<AACCharacter>();
	if (CharacterPawn == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("ControlledCharacter is nullptr"));
		return;
	}

	// Player에게 Sprint 요청
	AC_LOG(LogHY, Error, TEXT("PlayerController-Press Sprint Key"));
	CharacterPawn->Sprint(Value);
}

void AACPlayerControllerBase::HandleSprintEnd(const struct FInputActionValue& Value)
{
	AACCharacter* CharacterPawn = GetPawn<AACCharacter>();
	if (CharacterPawn == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("ControlledCharacter is nullptr"));
		return;
	}
	AC_LOG(LogHY, Error, TEXT("PlayerController-Release Sprint Key"));
	CharacterPawn->Sprint(Value);
}

void AACPlayerControllerBase::HandleSettingsClose(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	ControlledCharacter->SettingsClose();
}

void AACPlayerControllerBase::HandleSoundSetting(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	ControlledCharacter->SetSoundSetting();
}

void AACPlayerControllerBase::ChangeInputMode(EInputMode NewMode)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// 모든 매핑 컨텍스트 제거
		Subsystem->ClearAllMappings();

		// 모드에 따라 적절한 매핑 컨텍스트 추가
		if (NewMode == EInputMode::Sholder && DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		else if (NewMode == EInputMode::Settings && SettingsMappingContext)
		{
			Subsystem->AddMappingContext(SettingsMappingContext, 0);
		}
	}
}

void AACPlayerControllerBase::SoundSettingToggle(bool bVisible)
{
	if (SoundSettingWidget == nullptr)
	{
		AC_LOG(LogSY, Warning, TEXT("SoundSettingWidget is nullptr"));
		return;
	}

	if (bVisible == true)
	{
		AC_LOG(LogSY, Log, TEXT("open"));
		//SoundSettingWidget->AddToViewport();
		SoundSettingWidget->SetVisibility(ESlateVisibility::Visible);
		SoundSettingWidget->RefreshAudioDevices();

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
		//SoundSettingWidget->RemoveFromParent();
		SoundSettingWidget->SetVisibility(ESlateVisibility::Hidden);
		// 마우스 커서 끄기
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
		SetInputMode(FInputModeGameOnly());
	}

}


bool AACPlayerControllerBase::CanUseSkill() const
{
	AACCharacter* CharacterPawn = GetPawn<AACCharacter>();
	if (CharacterPawn == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("CharacterPawn is nullptr"));
		return false;
	}

	ECharacterState CharacterState = CharacterPawn->GetCharacterState();
	// 캐릭터의 상태가 None, Stun, Prison 상태일 경우 불가
	if (CharacterState == ECharacterState::None ||
		CharacterState == ECharacterState::Interact ||
		CharacterState == ECharacterState::OnInteract ||
		CharacterState == ECharacterState::Stun ||
		CharacterState == ECharacterState::Prison)
	{
		AC_LOG(LogHY, Error, TEXT("CharacterState is %s"), *UEnum::GetValueAsString(CharacterState));
		return false;
	}

	return true;
}

void AACPlayerControllerBase::Client_CleanupVoiceBeforeTravel_Implementation()
{
	// Voice 인터페이스 정리 (패킷 송수신 중지)
	IOnlineVoicePtr Voice = Online::GetVoiceInterface();
	if (Voice.IsValid())
	{
		Voice->ClearVoicePackets();
		Voice->RemoveAllRemoteTalkers();
		Voice->StopNetworkedVoice(0);
		Voice->UnregisterLocalTalker(0);
	}

	AC_LOG(LogSY, Log, TEXT("=== Client Voice 정리 완료 ==="));
}