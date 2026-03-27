
#include "ACMainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "ACTitlePlayerController.h"
#include "UI/Shop/ACShopWidget.h"
#include "AnimalCrime.h"
#include "ACAdvancedFriendsGameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/ACCharacter.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "UI/ACHUDWidget.h"
#include "UI/QuickSlot/ACQuickSlotWidget.h"
#include "UI/CCTV/ACCCTVWidget.h"
#include "UI/Interaction/ACInteractProgressWidget.h"
#include "UI/GameStart/ACRoleScreen.h"
#include "UI/GameResult/ACGameResultScreen.h"
#include "UI/Spectator/ACSpectatorScreen.h"
#include "UI/EscapeQuest/ACQuestTracker.h"
#include "ACPlayerState.h"
#include "ACMainGameState.h"
#include "ACMainGameMode.h"
#include "ACGameRuleManager.h"
#include "EngineUtils.h"
#include "OnlineSubsystem.h"
#include "Character/ACMafiaCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "UI/Ammo/ACAmmoWidget.h"
#include "Game/ACUIManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "CCTV/ACCCTVArea.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AACMainPlayerController::AACMainPlayerController()
{
	//탈출 스크린 로드
	static ConstructorHelpers::FClassFinder<UACSpectatorScreen> SpectatorScreenRef(TEXT("/Game/Project/UI/Spectator/WBP_SpectatorScreen.WBP_SpectatorScreen_C"));
	if (SpectatorScreenRef.Succeeded())
	{
		SpectatorScreenClass = SpectatorScreenRef.Class;
	}

	// HUD Class 대입.
	static ConstructorHelpers::FClassFinder<UACHUDWidget> ACHUDWidgetRef(TEXT("/Game/Project/UI/WBP_ACHUD.WBP_ACHUD_C"));
	if (ACHUDWidgetRef.Succeeded())
	{
		ACHUDWidgetClass = ACHUDWidgetRef.Class;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ACHUDWidgetRef is empty"));
	}

	//Role Screen 로드
	static ConstructorHelpers::FClassFinder<UACRoleScreen> RoleScreenRef(TEXT("/Game/Project/UI/GameStart/WBP_RoleScreen.WBP_RoleScreen_C"));
	if (RoleScreenRef.Succeeded())
	{
		RoleScreenClass = RoleScreenRef.Class;
	}

	//Game Result Screen 로드
	static ConstructorHelpers::FClassFinder<UACGameResultScreen> GameResultScreenRef(TEXT("/Game/Project/UI/GameResult/WBP_GameResultScreen.WBP_GameResultScreen_C"));
	if (GameResultScreenRef.Succeeded())
	{
		GameResultScreenClass = GameResultScreenRef.Class;
	}


	// ===== 입력 관련 로드 =====

	static ConstructorHelpers::FObjectFinder<UInputAction> DashActionRef(TEXT("/Game/Project/Input/Actions/IA_Dash.IA_Dash"));
	if (DashActionRef.Succeeded())
	{
		DashAction = DashActionRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> EscapeActionRef(TEXT("/Game/Project/Input/Actions/IA_Escape.IA_Escape"));
	if (EscapeActionRef.Succeeded())
	{
		EscapeAction = EscapeActionRef.Object;
	}

	// ===== 퀵슬롯 Input Action 로드 (하나만) =====
	static ConstructorHelpers::FObjectFinder<UInputAction> QuickSlotActionRef(TEXT("/Game/Project/Input/Actions/IA_QuickSlot.IA_QuickSlot"));
	if (QuickSlotActionRef.Succeeded())
	{
		QuickSlotAction = QuickSlotActionRef.Object;
	}

	// ===== 관전자 입력 로드 =====
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> SpectatorMappingContextRef(TEXT("/Game/Project/Input/IMC_Spectator.IMC_Spectator"));
	if (SpectatorMappingContextRef.Succeeded())
	{
		SpectatorMappingContext = SpectatorMappingContextRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> SpectatorChangeActionRef(TEXT("/Game/Project/Input/Actions/IA_SpectatorChange.IA_SpectatorChange"));
	if (SpectatorChangeActionRef.Succeeded())
	{
		SpectatorChangeAction = SpectatorChangeActionRef.Object;
	}


	// ===== Zoom ====
	static ConstructorHelpers::FObjectFinder<UInputAction> ZoomActionRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Project/Input/Actions/IA_Zoom.IA_Zoom'"));
	if (ZoomActionRef.Succeeded())
	{
		ZoomAction = ZoomActionRef.Object;
	}
	
	// ===== 게임 시간 ====
	static ConstructorHelpers::FObjectFinder<UInputAction> TimerUpActionRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Project/Input/Actions/IA_TimeUp.IA_TimeUp'"));
	if (TimerUpActionRef.Succeeded())
	{
		TimerUpAction = TimerUpActionRef.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction> TimerDownActionRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Project/Input/Actions/IA_TimeDown.IA_TimeDown'"));
	if (TimerDownActionRef.Succeeded())
	{
		TimerDownAction = TimerDownActionRef.Object;
	}

	// ===== 핸드폰 관련 로드 =====
	static ConstructorHelpers::FObjectFinder<UInputAction> PhoneActionRef(TEXT("/Game/Project/Input/Actions/IA_Phone.IA_Phone"));
	if (PhoneActionRef.Succeeded())
	{
		PhoneAction = PhoneActionRef.Object;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> PhoneWidgetRef(TEXT("/Game/Project/UI/Phone/WBP_Phone.WBP_Phone_C"));
	if (PhoneWidgetRef.Succeeded())
	{
		PhoneWidgetClass = PhoneWidgetRef.Class;
	}

	// UI 매니저 컴포넌트 생성
	UIManager = CreateDefaultSubobject<UACUIManagerComponent>(TEXT("UIManager"));
}

void AACMainPlayerController::PostInitializeComponents()
{
	AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::PostInitializeComponents();
	AC_LOG(LogHY, Warning, TEXT("End"));
}

void AACMainPlayerController::PostNetInit()
{
	AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::PostNetInit();
	AC_LOG(LogHY, Warning, TEXT("End"));
}

void AACMainPlayerController::BeginPlay()
{
	AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::BeginPlay();

	//ConsoleCommand(TEXT("show Collision"));
	//ConsoleCommand(TEXT("Stat FPS"));
	UE_LOG(LogTemp, Warning, TEXT("AACMainPlayerController BeginPlay"));

	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);
	if (IsLocalController() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("IsLocalController false"));
		return;
	}

	ACHUDWidget = CreateWidget<UACHUDWidget>(this, ACHUDWidgetClass);
	if (ACHUDWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ACHUDWidget is empty"));
		return;
	}

	// 뭐하는 함수인지 알아보기.
	ACHUDWidget->AddToViewport();

	ACHUDWidget->BindGameState();

	//if (HasAuthority())
	//{
		// 서버와 클라이언트 모두 바인딩 필요
	ACHUDWidget->BindPlayerState();
	//}

	ZoomOut();
	ACHUDWidget->HideSprintUI();
	ACHUDWidget->WBP_Ammo->UpdateAmmo(0);

	// 서버(리슨 서버 호스트)인 경우에만 여기서 RoleScreen 표시
	if (IsLocalController() && HasAuthority())
	{
		ScreenSetRole();
	}
	AC_LOG(LogHY, Warning, TEXT("End"));

	bReplicates = true;
}

void AACMainPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AACMainPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AACMainPlayerController, bZoomFlag);
}

void AACMainPlayerController::TimeUp()
{
	AC_LOG(LogHY, Error, TEXT("TimeUp !!!"));
	AACMainGameMode* GameMode = GetWorld()->GetAuthGameMode<AACMainGameMode>();
	if (GameMode == nullptr)
	{
		return;
	}

	AC_LOG(LogHY, Error, TEXT("TimeUp"));
	GameMode->GetGameRuleManager()->RemainTimeUp(60);
}

void AACMainPlayerController::TimeDown()
{
	AC_LOG(LogHY, Error, TEXT("TimeDown !!!"));
	AACMainGameMode* GameMode = GetWorld()->GetAuthGameMode<AACMainGameMode>();
	if (GameMode == nullptr)
	{
		return;
	}

	AC_LOG(LogHY, Error, TEXT("TimeDown"));
	GameMode->GetGameRuleManager()->RemainTimeDown(60);
}

void AACMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Enhanced Input Component 획득
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("EnhancedInputComponent is nullptr"));
		return;
	}

	// 입력 액션 바인딩

	// 캐릭터 스킬 - Dash
	if (DashAction)
	{
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AACMainPlayerController::HandleDash);
	}
	// 캐릭터 스킬 - Escape
	if (EscapeAction)
	{
		EnhancedInputComponent->BindAction(EscapeAction, ETriggerEvent::Started, this, &AACMainPlayerController::HandleEscape);
	}

	if (SettingsCloseAction)
	{
		EnhancedInputComponent->BindAction(SettingsCloseAction, ETriggerEvent::Started, this, &AACMainPlayerController::HandleSettingsClose);
	}

	if (QuickSlotAction)
	{
		EnhancedInputComponent->BindAction(QuickSlotAction, ETriggerEvent::Started, this, &AACMainPlayerController::HandleQuickSlot);
	}

	if (SpectatorChangeAction)
	{
		EnhancedInputComponent->BindAction(SpectatorChangeAction, ETriggerEvent::Started, this, &AACMainPlayerController::HandleSpectatorChange);
	}

	if (ZoomAction)
	{
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Started, this, &AACMainPlayerController::ZoomIn);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Completed, this, &AACMainPlayerController::ZoomOut);
	}
	
	if (TimerUpAction)
	{
		EnhancedInputComponent->BindAction(TimerUpAction, ETriggerEvent::Started, this, &AACMainPlayerController::TimeUp);
	}
	
	if (TimerDownAction)
	{
		EnhancedInputComponent->BindAction(TimerDownAction, ETriggerEvent::Started, this, &AACMainPlayerController::TimeDown);
	}

	if (PhoneAction)
	{
		EnhancedInputComponent->BindAction(PhoneAction, ETriggerEvent::Started, this, &AACMainPlayerController::HandlePhone);
	}
}

void AACMainPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ACHUDWidget->BindPlayerState();

	ScreenSetRole();
}

// ===== 입력 핸들러 구현 =====

void AACMainPlayerController::HandleAttack(const FInputActionValue& Value)
{
	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	if (bZoomFlag == false)
	{
		ControlledCharacter->Attack();
	}
	else
	{
		AC_LOG(LogHY, Error, TEXT("빵야빵야"));
		ControlledCharacter->ServerPlayShootMontage();
		//ControlledCharacter->FireHitscan();
	}
}

void AACMainPlayerController::HandleSpectatorChange(const FInputActionValue& Value)
{
	AC_LOG(LogSY, Log, TEXT("Spectator Change!!"));

	if (IsLocalController() == false)
	{
		return;
	}

	//관전 대상 변경
	ServerSwitchToNextSpectateTarget();
}

#pragma region 캐릭터 스킬 Handler

void AACMainPlayerController::HandleDash(const FInputActionValue& Value)
{
	if (CanUseSkill() == false)
	{
		AC_LOG(LogHY, Error, TEXT("CanUseSkill is false"));
		return;
	}

	AACCharacter* CharacterPawn = GetPawn<AACCharacter>();
	if (CharacterPawn == nullptr)
	{
		// 사실 무조건 통과함. - CanUseSkill 에서 확인
		AC_LOG(LogHY, Error, TEXT("ControlledCharacter is nullptr"));
		return;
	}

	// Player에게 Dash 요청
	CharacterPawn->Dash(Value);
}

void AACMainPlayerController::HandleEscape(const FInputActionValue& Value)
{
	bool InputFlag = Value.Get<bool>();
	if (InputFlag == false)
	{
		AC_LOG(LogHY, Error, TEXT("InputFlag is false"));
		return;
	}

	if (CanUseEscapeSkill() == false)
	{
		AC_LOG(LogHY, Error, TEXT("CanUseEscapeSkill is false"));
		return;
	}


	AACMafiaCharacter* MafiaPawn = GetPawn<AACMafiaCharacter>();
	if (MafiaPawn == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("ControlledCharacter is nullptr"));
		return;
	}
	AC_LOG(LogHY, Error, TEXT("PlayerController-Press Escape Key"));
	MafiaPawn->ExcuteEscape();
}

#pragma endregion

void AACMainPlayerController::HandleQuickSlot(const FInputActionValue& Value)
{
	if (ACHUDWidget == nullptr || ACHUDWidget->WBP_QuickSlot == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleQuickSlot: QuickSlot widget is null"));
		return;
	}

	// Value에서 슬롯 번호 가져오기 (1, 2, 3, ...)
	int32 SlotIndex = FMath::RoundToInt(Value.Get<float>());

	UE_LOG(LogTemp, Log, TEXT("QuickSlot pressed: %d"), SlotIndex);

	// 통합된 함수 호출
	ACHUDWidget->WBP_QuickSlot->ToggleSlotEquip(SlotIndex);
}

void AACMainPlayerController::HandlePhone(const FInputActionValue& Value)
{
	// 경찰만 핸드폰 사용 가능
	AACPlayerState* PS = GetPlayerState<AACPlayerState>();
	if (PS == nullptr || PS->PlayerRole != EPlayerRole::Police)
	{
		return;
	}

	// 마우스 휠 값 가져오기 (1.0 = 위로, -1.0 = 아래로)
	float WheelValue = Value.Get<float>();

	if (WheelValue > 0.0f)
	{
		// 휠 위로 = 핸드폰 열기
		if (CurrentPhoneWidget == nullptr || CurrentPhoneWidget->IsInViewport() == false)
		{
			if (PhoneWidgetClass != nullptr)
			{
				CurrentPhoneWidget = CreateWidget<UUserWidget>(this, PhoneWidgetClass);
				if (CurrentPhoneWidget != nullptr)
				{
					CurrentPhoneWidget->AddToViewport();

					// 입력 모드를 게임+UI로 변경
					FInputModeGameAndUI InputMode;
					InputMode.SetWidgetToFocus(CurrentPhoneWidget->TakeWidget());
					InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					SetInputMode(InputMode);

					SetShowMouseCursor(true);
				}
			}
		}
	}
	else if (WheelValue < 0.0f)
	{
		// 휠 아래로 = 핸드폰 닫기
		ClosePhone();
	}
}

bool AACMainPlayerController::CanUseEscapeSkill() const
{
	AACMafiaCharacter* MafiaPawn = GetPawn<AACMafiaCharacter>();
	if (MafiaPawn == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("CharacterPawn is nullptr"));
		return false;
	}

	ECharacterState CharacterState = MafiaPawn->GetCharacterState();
	// 캐릭터의 상태가 None, Stun, Prison 상태일 경우 불가
	if (CharacterState != ECharacterState::OnInteract)
	{
		AC_LOG(LogHY, Error, TEXT("CharacterState is %s"), *UEnum::GetValueAsString(CharacterState));
		return false;
	}

	ECharacterState PrevCharacterState = MafiaPawn->GetPrevCharacterState();
	if (PrevCharacterState == ECharacterState::Stun)
	{
		AC_LOG(LogHY, Error, TEXT("PrevCharacterState was %s"), *UEnum::GetValueAsString(PrevCharacterState));
		return false;
	}
	
	return true;
}

void AACMainPlayerController::ChangeInputMode(EInputMode NewMode)
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
		else if (NewMode == EInputMode::Spectator && SpectatorMappingContext)
		{
			Subsystem->AddMappingContext(SpectatorMappingContext, 0);
		}
		else
		{
			AC_LOG(LogSY, Log, TEXT("ChangeMode fail"));
		}
	}
}

void AACMainPlayerController::ClientOnEscapeSuccess_Implementation()
{
	if (ACHUDWidget == nullptr)
	{
		return;
	}
	ACHUDWidget->RemoveFromParent();
	ShowNotification(FText::FromString(TEXT("탈출")), ENotificationType::None);

	// IMC 변경
	ChangeInputMode(EInputMode::Spectator);
}

void AACMainPlayerController::ClientToggleShopWidget_Implementation(TSubclassOf<class UACShopWidget> WidgetClass)
{
	if (WidgetClass == nullptr)
	{
		UE_LOG(LogHG, Error, TEXT("ClientToggleShopWidget: WidgetClass is null"));
		return;
	}

	// ===== Phone이 열려있으면 먼저 닫기 =====
	ClosePhone();

	// 이미 위젯이 열려있으면 닫기
	if (CurrentShopWidget != nullptr && CurrentShopWidget->IsInViewport())
	{
		CloseShop();
	}
	else
	{
		// ===== 상점 열기 =====
		UACShopWidget* ShopWidget = CreateWidget<UACShopWidget>(GetWorld(), WidgetClass);

		if (ShopWidget != nullptr)
		{
			ShopWidget->AddToViewport();
			CurrentShopWidget = ShopWidget;

			// 델리게이트 바인딩
			ShopWidget->OnCloseRequested.AddDynamic(this, &AACMainPlayerController::CloseShop);

			// 상점용 카메라로 전환
			SetShopCamera();

			// 입력모드를 게임,UI로 변경
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(ShopWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(false); // 클릭해도 마우스 커서 계속 보이도록
			SetInputMode(InputMode);

			SetShowMouseCursor(true); // 마우스 커서 보이도록

			// 상점의 슬롯들 로드
			ShopWidget->LoadAndCreateSlots(TEXT("/Game/Project/Item/"));

			// 입력 모드 변경
			ChangeInputMode(EInputMode::Settings);
		}
	}
}

void AACMainPlayerController::CloseShop_Implementation()
{
	if (CurrentShopWidget == nullptr || CurrentShopWidget->IsInViewport() == false)
	{
		// 이미 닫혀있음
		return;
	}

	// ===== 상점 닫기 =====
	CurrentShopWidget->RemoveFromParent();
	CurrentShopWidget = nullptr;

	// 카메라 복원
	RestoreOriginalCamera();

	// 마우스 숨기기
	SetShowMouseCursor(false);

	// ===== 입력 모드를 게임 모드로 전환 =====
	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	SetInputMode(InputMode);

	// 입력 모드 변경
	ChangeInputMode(EInputMode::Sholder);
}

void AACMainPlayerController::CloseCCTV_Implementation()
{
	if (CurrentCCTVWidget == nullptr || CurrentCCTVWidget->IsInViewport() == false)
	{
		return;
	}

	// 클라이언트도 SceneCapture 비활성화
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AACCCTVArea::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		AACCCTVArea* CCTVArea = Cast<AACCCTVArea>(FoundActors[0]);
		if (CCTVArea != nullptr)
		{
			CCTVArea->SetSceneCaptureActive(false);
		}
	}

	// ===== CCTV 닫기 =====
	CurrentCCTVWidget->RemoveFromParent();
	CurrentCCTVWidget = nullptr;

	SetShowMouseCursor(false);

	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	SetInputMode(InputMode);

	ChangeInputMode(EInputMode::Sholder);
}

void AACMainPlayerController::SetShopCamera()
{
	if (bShopCameraActive == true)
	{
		// 이미 상점 카메라 모드
		return;
	}

	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	// CameraBoom 가져오기
	USpringArmComponent* CameraBoom = ControlledCharacter->FindComponentByClass<USpringArmComponent>();
	if (CameraBoom == nullptr)
	{
		UE_LOG(LogHG, Error, TEXT("CameraBoom not found on Character"));
		return;
	}

	// ===== 현재 카메라 상태 저장 =====
	SavedControlRotation = GetControlRotation();
	SavedTargetArmLength = CameraBoom->TargetArmLength;
	SavedTargetOffset = CameraBoom->TargetOffset;
	SavedRelativeLocation = CameraBoom->GetRelativeLocation();
	SavedRelativeRotation = CameraBoom->GetRelativeRotation();
	bSavedUsePawnControlRotation = CameraBoom->bUsePawnControlRotation;

	// ===== 월드 기준 고정 각도로 카메라 설정 =====

	// CameraBoom이 컨트롤러 회전을 따라가지 않도록 설정
	CameraBoom->bUsePawnControlRotation = false;

	// 캐릭터 정면(월드 기준 북쪽)에서 약간 위에서 보도록 설정
	FRotator WorldFixedRotation = FRotator(-15.f, 0.f, 0.f);
	SetControlRotation(WorldFixedRotation);

	// CameraBoom의 로컬 회전을 설정 (캐릭터를 정면에서 봄)
	CameraBoom->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	CameraBoom->SetRelativeLocation(FVector(100.0f, 100.0f, 0.f));

	// 카메라 거리 및 오프셋 조정
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->TargetOffset = FVector(0.f, 0.f, 50.f);

	bShopCameraActive = true;
}

void AACMainPlayerController::RestoreOriginalCamera()
{
	if (bShopCameraActive == false)
	{
		return;  // 상점 카메라 모드가 아님
	}

	AACCharacter* ControlledCharacter = GetPawn<AACCharacter>();
	if (ControlledCharacter == nullptr)
	{
		return;
	}

	// CameraBoom 가져오기
	USpringArmComponent* CameraBoom = ControlledCharacter->FindComponentByClass<USpringArmComponent>();
	if (CameraBoom == nullptr)
	{
		return;
	}

	// ===== 원래 카메라 상태 복원 =====

	// 컨트롤러 회전 복원
	SetControlRotation(SavedControlRotation);

	// CameraBoom 설정 복원
	CameraBoom->bUsePawnControlRotation = bSavedUsePawnControlRotation;
	CameraBoom->SetRelativeRotation(SavedRelativeRotation);
	CameraBoom->SetRelativeLocation(SavedRelativeLocation);
	CameraBoom->TargetArmLength = SavedTargetArmLength;
	CameraBoom->TargetOffset = SavedTargetOffset;

	bShopCameraActive = false;
}

void AACMainPlayerController::OnRoleFadeInFinished()
{
	AC_LOG(LogSY, Log, TEXT("FadeOut"));
	RoleScreen->RemoveFromParent();
}

void AACMainPlayerController::ScreenSetRole()
{

	AACPlayerState* PS = GetPlayerState<AACPlayerState>();
	if (PS == nullptr)
	{
		return;
	}

	if (PS->PlayerRole == EPlayerRole::None)
	{
		AC_LOG(LogSY, Warning, TEXT("PlayerRole not replicated yet, skipping ScreenSetRole"));
		return;
	}

	// 대체 화면 제거
	UACAdvancedFriendsGameInstance* GI = GetGameInstance<UACAdvancedFriendsGameInstance>();
	if (GI == nullptr)
	{
		return;
	}
	GI->HideTransitionScreen();


	RoleScreen = CreateWidget<UACRoleScreen>(this, RoleScreenClass);
	if (RoleScreen == nullptr)
	{
		return;
	}


	RoleScreen->AddToViewport();
	RoleScreen->OnFadeOutFinished.AddDynamic(this, &AACMainPlayerController::OnRoleFadeInFinished);
	RoleScreen->SetRole(PS->PlayerRole);

	if (PS->PlayerRole == EPlayerRole::Mafia)
	{
		if (ACHUDWidget == nullptr || ACHUDWidget->QuestTracker == nullptr)
		{
			AC_LOG(LogSY, Log, TEXT("ACHUDWidget or QuestTracker is nullptr"));
			return;
		}
		ACHUDWidget->QuestTracker->SetVisibility(ESlateVisibility::Visible);
	}
	AC_LOG(LogSY, Log, TEXT("Set Role!"));
}

void AACMainPlayerController::Client_ShowGameResult_Implementation(EGameEndType GameEndType)
{
	if (GameResultScreenClass == nullptr)
	{
		AC_LOG(LogSY, Error, TEXT("GameResultScreenClass is nullptr"));
		return;
	}
	UACGameResultScreen* GameResultScreen = CreateWidget<UACGameResultScreen>(this, GameResultScreenClass);
	if (GameResultScreen == nullptr)
	{
		AC_LOG(LogSY, Error, TEXT("GameResultWidget is nullptr"));
		return;
	}
	GameResultScreen->AddToViewport();
	GameResultScreen->SetGameResult(GameEndType);
}

void AACMainPlayerController::ZoomIn()
{
	AACCharacter* CharacterPawn = GetPawn<AACCharacter>();
	if (CharacterPawn == nullptr)
	{
		AC_LOG(LogHY, Log, TEXT("CharacterPawn is nullptr"));
		return;
	}

	if (CharacterPawn->IsHoldingGun() == false)
	{
		AC_LOG(LogHY, Log, TEXT("No Gun In Hand"));
		return;
	}

	if (CharacterPawn->CanZoomIn() == false)
	{
		AC_LOG(LogHY, Log, TEXT("줌이 안된데"));
		return;
	}
	Server_Zoom(true);
	
	//CharacterPawn->ServerPlayZoomIn();
	CharacterPawn->SetZoomInState(true);
	AC_LOG(LogHY, Error, TEXT("ZoomIn %d"), bZoomFlag);

	UCameraComponent* FollowCamera = CharacterPawn->GetFollowCamera();
	if (FollowCamera == nullptr)
	{
		AC_LOG(LogHY, Log, TEXT("FollowCamera is nullptr"));
		return;
	}
	UCameraComponent* GunCamera = CharacterPawn->GetGunCamera();
	if (GunCamera == nullptr)
	{
		AC_LOG(LogHY, Log, TEXT("GunCamera is nullptr"));
		return;
	}

	// 기존 카메라 대신 총의 위치 카메라로 변경.
	FollowCamera->Deactivate();
	GunCamera->Activate();
	USkeletalMeshComponent* Mesh = CharacterPawn->GetMesh();
	if (Mesh)
	{
		Mesh->SetHiddenInGame(true);
		CharacterPawn->GetHeadMesh()->SetHiddenInGame(true);
		CharacterPawn->GetFaceMesh()->SetHiddenInGame(true);
		CharacterPawn->GetTopMesh()->SetHiddenInGame(true);
		CharacterPawn->GetBottomMesh()->SetHiddenInGame(true);
		CharacterPawn->GetFaceAccMesh()->SetHiddenInGame(true);
		CharacterPawn->GetShoesMesh()->SetHiddenInGame(true);

		TArray<USceneComponent*> Childrens;
		Mesh->GetChildrenComponents(true, Childrens);

		for (USceneComponent* Child : Childrens)
		{
			UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Child);
			if (!StaticMeshComp) continue;

			// RightHand 소켓에 붙어 있는지 확인
			if (StaticMeshComp->GetAttachSocketName() == TEXT("RightHandPistolSocket"))
			{
				StaticMeshComp->SetHiddenInGame(true);
			}
		}

	}

	ACHUDWidget->ZoomInState();
}

void AACMainPlayerController::ZoomOut()
{
	AC_LOG(LogHY, Error, TEXT("ZoomOut %d"), bZoomFlag);

	AACCharacter* CharacterPawn = GetPawn<AACCharacter>();
	if (CharacterPawn == nullptr)
	{
		AC_LOG(LogHY, Log, TEXT("CharacterPawn is nullptr"));
		return;
	}

	UCameraComponent* FollowCamera = CharacterPawn->GetFollowCamera();
	if (FollowCamera == nullptr)
	{
		AC_LOG(LogHY, Log, TEXT("FollowCamera is nullptr"));
		return;
	}

	UCameraComponent* GunCamera = CharacterPawn->GetGunCamera();
	if (GunCamera == nullptr)
	{
		AC_LOG(LogHY, Log, TEXT("GunCamera is nullptr"));
		return;
	}

	//CharacterPawn->ServerPlayZoomOut();
	CharacterPawn->SetZoomInState(false);
	Server_Zoom(false);

	FollowCamera->Activate();
	GunCamera->Deactivate();
	USkeletalMeshComponent* Mesh = CharacterPawn->GetMesh();
	if (Mesh)
	{
		Mesh->SetOwnerNoSee(false);
		CharacterPawn->GetHeadMesh()->SetHiddenInGame(false);
		CharacterPawn->GetFaceMesh()->SetHiddenInGame(false);
		CharacterPawn->GetTopMesh()->SetHiddenInGame(false);
		CharacterPawn->GetBottomMesh()->SetHiddenInGame(false);
		CharacterPawn->GetFaceAccMesh()->SetHiddenInGame(false);
		CharacterPawn->GetShoesMesh()->SetHiddenInGame(false);
		Mesh->SetHiddenInGame(false);

		TArray<USceneComponent*> Childrens;
		Mesh->GetChildrenComponents(true, Childrens);

		for (USceneComponent* Child : Childrens)
		{
			UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Child);
			if (!StaticMeshComp) continue;

			// RightHand 소켓에 붙어 있는지 확인
			if (StaticMeshComp->GetAttachSocketName() == TEXT("RightHandPistolSocket"))
			{
				StaticMeshComp->SetHiddenInGame(false);
			}
		}
	}

	ACHUDWidget->ZoomOutState();
}

void AACMainPlayerController::Server_Zoom_Implementation(bool NewZoomFlag)
{
	bZoomFlag = NewZoomFlag;
	OnRep_Zoom();
}

void AACMainPlayerController::OnRep_Zoom()
{
	AACCharacter* CharacterPawn = GetPawn<AACCharacter>();
	if (CharacterPawn == nullptr)
	{
		AC_LOG(LogHY, Log, TEXT("CharacterPawn is nullptr"));
		return;
	}

	// Zoom In 상태
	if (bZoomFlag == true)
	{
		UCharacterMovementComponent* CharacterMoveComp = CharacterPawn->GetCharacterMovement();
		if (CharacterPawn)
		{
			CharacterMoveComp->bOrientRotationToMovement = false;
		}

		// 컨트롤러 회전(Yaw)을 사용
		CharacterPawn->bUseControllerRotationYaw = true;
	}
	else
	{
		UCharacterMovementComponent* CharacterMoveComp = CharacterPawn->GetCharacterMovement();
		if (CharacterPawn)
		{
			CharacterMoveComp->bOrientRotationToMovement = true;
		}

		// 컨트롤러 회전(Yaw)을 사용
		CharacterPawn->bUseControllerRotationYaw = false;
	}
}

void AACMainPlayerController::ClientNotifySpectateTargetRemoved_Implementation(APawn* RemovedPawn)
{
	if (IsLocalController() == false)
	{
		return;
	}

	// 현재 관전 중인 대상 확인
	AActor* CurrentViewTarget = GetViewTarget();

	// 제거된 폰이 현재 관전 중인 대상인 경우
	if (CurrentViewTarget == RemovedPawn)
	{
		AC_LOG(LogSY, Log, TEXT("Current spectate target removed. Requesting auto-switch to next target."));

		// 서버에 다음 대상으로 전환 요청
		ServerSwitchToNextSpectateTarget();
	}
	else
	{
		AC_LOG(LogSY, Log, TEXT("Removed pawn is not current view target. No action needed."));
	}
}

void AACMainPlayerController::ServerSwitchToNextSpectateTarget_Implementation()
{
	AACMainGameState* GS = GetWorld()->GetGameState<AACMainGameState>();
	if (GS == nullptr)
	{
		AC_LOG(LogSY, Error, TEXT("GameState is nullptr"));
		return;
	}

	const TArray<TObjectPtr<APawn>>& Targets = GS->GetSpectatablePawns();

	// 관전 가능한 대상이 없는 경우
	if (Targets.Num() == 0)
	{
		AC_LOG(LogSY, Warning, TEXT("No spectatable targets available"));
		CurrentSpectateIndex = INDEX_NONE;
		return;
	}

	// 현재 인덱스 초기화
	if (CurrentSpectateIndex == INDEX_NONE || Targets.IsValidIndex(CurrentSpectateIndex) == false)
	{
		CurrentSpectateIndex = -1;
	}

	// 다음 유효한 대상 찾기
	int32 SearchCount = 0;

	do
	{
		// 다음 인덱스 이동
		CurrentSpectateIndex = (CurrentSpectateIndex + 1) % Targets.Num();
		SearchCount++;

		APawn* NextPawn = Targets[CurrentSpectateIndex];

		// 유효한 폰인지 확인
		if (NextPawn != nullptr && IsValid(NextPawn) == true)
		{
			// ViewTarget 설정
			SetViewTargetWithBlend(NextPawn, 0.0f);

			// 클라이언트에 UI 업데이트 알림
			AACPlayerState* TargetPS = NextPawn->GetPlayerState<AACPlayerState>();
			if (TargetPS == nullptr)
			{
				return;
			}
			ClientOnSpectateTargetChanged(TargetPS);

			return;
		}

		// 모든 대상을 확인했는데 유효한 폰이 없는 경우
		if (SearchCount >= Targets.Num())
		{
			AC_LOG(LogSY, Warning, TEXT("No valid spectatable pawns found"));
			CurrentSpectateIndex = INDEX_NONE;
			return;
		}

	} while (SearchCount < Targets.Num());
}


void AACMainPlayerController::ServerStartSpectateOtherPlayer_Implementation()
{
	APawn* MyPawn = GetPawn();
	if (MyPawn == nullptr)
	{
		return;
	}

	MyPawn->SetActorHiddenInGame(true);
	MyPawn->SetActorEnableCollision(false);

	//관전 대상 선택
	//ServerSwitchToNextSpectateTarget();

	//게임종료 조건 체크
	AACMainGameMode* GM = GetWorld()->GetAuthGameMode<AACMainGameMode>();
	if (GM == nullptr)
	{
		return;
	}

	GM->GetGameRuleManager()->CheckGameEndCondition();
}

void AACMainPlayerController::ClientToggleCCTVWidget_Implementation(TSubclassOf<class UACCCTVWidget> WidgetClass)
{
	if (WidgetClass == nullptr)
	{
		UE_LOG(LogHG, Error, TEXT("ClientToggleCCTVWidget: WidgetClass is null"));
		return;
	}

	// ===== Phone이 열려있으면 먼저 닫기 =====
	ClosePhone();

	// 이미 위젯이 열려있으면 닫기
	if (CurrentCCTVWidget != nullptr && CurrentCCTVWidget->IsInViewport())
	{
		CloseCCTV();
	}
	else
	{
		// ===== CCTV 열기 =====
		UACCCTVWidget* CCTVWidget = CreateWidget<UACCCTVWidget>(GetWorld(), WidgetClass);

		if (CCTVWidget != nullptr)
		{
			CCTVWidget->AddToViewport();
			CurrentCCTVWidget = CCTVWidget;

			//  클라이언트에서도 SceneCapture 활성화
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AACCCTVArea::StaticClass(), FoundActors);
			if (FoundActors.Num() > 0)
			{
				AACCCTVArea* CCTVArea = Cast<AACCCTVArea>(FoundActors[0]);
				if (CCTVArea != nullptr)
				{
					CCTVArea->SetSceneCaptureActive(true);
				}
			}

			// 입력모드를 게임,UI로 변경
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(CCTVWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(false);
			SetInputMode(InputMode);

			SetShowMouseCursor(true);

			ChangeInputMode(EInputMode::Settings);
		}
	}
}

void AACMainPlayerController::ShowInteractProgress(const FString& InteractionName)
{
	//UE_LOG(LogSW, Log, TEXT("Showing"));
	if (ACHUDWidget == nullptr)
	{
		return;
	}
	if (ACHUDWidget->WBP_InteractProgress == nullptr)
	{
		return;
	}

	ACHUDWidget->WBP_InteractProgress->SetInteractionText(InteractionName);
	ACHUDWidget->WBP_InteractProgress->SetProgress(0.f);
	ACHUDWidget->WBP_InteractProgress->ShowWidget();
}

void AACMainPlayerController::UpdateInteractProgress(float Progress)
{
	if (ACHUDWidget == nullptr)
	{
		return;
	}
	if (ACHUDWidget->WBP_InteractProgress == nullptr)
	{
		return;
	}

	ACHUDWidget->WBP_InteractProgress->SetProgress(Progress);
}

void AACMainPlayerController::HideInteractProgress()
{
	//UE_LOG(LogSW, Log, TEXT("Hiding"));
	if (ACHUDWidget == nullptr)
	{
		return;
	}
	if (ACHUDWidget->WBP_InteractProgress == nullptr)
	{
		return;
	}

	ACHUDWidget->WBP_InteractProgress->HideWidget();
}

void AACMainPlayerController::ClosePhone()
{
	if (CurrentPhoneWidget && CurrentPhoneWidget->IsInViewport())
	{
		CurrentPhoneWidget->RemoveFromParent();
		CurrentPhoneWidget = nullptr;

		// 입력 모드를 게임 모드로 복원
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		SetShowMouseCursor(false);
	}
}
void AACMainPlayerController::UpdateAmmoUI(int32 Ammo)
{
	ACHUDWidget->HandleAmmoChanged(Ammo);
}

void AACMainPlayerController::ClientOnSpectateTargetChanged_Implementation(AACPlayerState* NewTargetPS)
{
	//만약 SpectatorScreen 이 없다면 생성
	if (SpectatorScreen == nullptr)
	{
		SpectatorScreen = CreateWidget<UACSpectatorScreen>(this, SpectatorScreenClass);
		if (SpectatorScreen == nullptr)
		{
			return;
		}
		SpectatorScreen->AddToViewport();
		SpectatorScreen->BindGameState();
	}
	//UI 수정
	SpectatorScreen->SetFriend(NewTargetPS);
}

void AACMainPlayerController::ShowNotification(const FText& Text, ENotificationType NotificationType)
{
	if (IsLocalController() == false)
	{
		return;
	}

	if (UIManager == nullptr)
	{
		AC_LOG(LogSY, Error, TEXT("UIManager is nullptr"));
	}
	UIManager->ShowNotification(Text, NotificationType);
}

void AACMainPlayerController::Client_ShowNotification_Implementation(const FText& Message, ENotificationType NotificationType)
{
	ShowNotification(Message, NotificationType);
}

void AACMainPlayerController::SetHasWalkyTalky(bool bInHasWalkyTalky)
{
	if (bHasWalkyTalky == bInHasWalkyTalky)
	{
		return; // 이미 같은 값이면 리턴
	}

	bHasWalkyTalky = bInHasWalkyTalky;

	// 델리게이트 브로드캐스트 (로컬에서만)
	OnWalkyTalkyChanged.Broadcast(bHasWalkyTalky);

	UE_LOG(LogHG, Log, TEXT("[Local] 무전기 소지 여부: %s"), bHasWalkyTalky ? TEXT("O") : TEXT("X"));
}

void AACMainPlayerController::SetHasContraband(bool bInHasContraband)
{
	if (bHasContraband == bInHasContraband)
	{
		return;
	}

	bHasContraband = bInHasContraband;

	// 델리게이트 브로드캐스트 (로컬에서만)
	OnContrabandChanged.Broadcast(bHasContraband);

	UE_LOG(LogHG, Log, TEXT("[Local] 밀수품 소지 여부: %s"), bHasContraband ? TEXT("O") : TEXT("X"));
}