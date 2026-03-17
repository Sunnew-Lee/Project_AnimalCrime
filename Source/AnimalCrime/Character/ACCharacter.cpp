#include "ACCharacter.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NavigationSystem.h"
#include "AI/ACCitizenAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Item/ACEscapeMissionBomb.h"
#include "Kismet/GameplayStatics.h"

#include "Component/ACInteractableComponent.h"
#include "ACCharacterAnimInstance.h"
#include "Components/WidgetComponent.h"
#include "UI/Interaction/ACInteractionInfoWidget.h"
#include "Interaction/ACInteractionData.h"
#include "Game/ACInteractionSubsystem.h"
#include "Interaction/ACInteractionDatabase.h"
#include "Prison/ACPrisonBase.h"

#include "AnimalCrime.h"
#include "NetworkMessage.h"

#include "Component/ACShopComponent.h"
#include "UI/Shop/ACShopWidget.h"
#include "Game/ACMainPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Component/ACDestroyableStatComponent.h"
#include "Component/ACMoneyComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Net/UnrealNetwork.h"
#include "Objects/MoneyData.h"

#include "Game/ACPlayerState.h"
#include "Game/ACPlayerControllerBase.h"
#include "Game/ACAdvancedFriendsGameInstance.h"
#include "Skill/ACSkillData.h"
#include "Voice/ACVOIPTalker.h"
#include "Sound/SoundBase.h"
#include "UI/ACHUDWidget.h"
#include "Materials/MaterialInterface.h"
#include "Net/VoiceConfig.h"
#include "EngineUtils.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Objects/ACGunBase.h"

AACCharacter::AACCharacter()
{
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	//캐릭터 무브먼트
	auto Move = GetCharacterMovement();
	Move->MaxWalkSpeed = 300.f;
	Move->JumpZVelocity = 500.f;
	Move->AirControl = 0.3f;
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 500.f, 0.f);

	//스켈레탈 메시
	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetSkeletalMesh(
		LoadObject<USkeletalMesh>(
			nullptr, TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Body_010.SK_Body_010")));
	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	MeshComp->SetReceivesDecals(false);

	static ConstructorHelpers::FClassFinder<UACCharacterAnimInstance> AnimRef(
		TEXT("/Script/Engine.AnimBlueprint'/Game/Project/Character/ABP_ACPlayer.ABP_ACPlayer_C'"));
	if (AnimRef.Succeeded())
	{
		MeshComp->SetAnimInstanceClass(AnimRef.Class);
	}

	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Head"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> HeadMeshRef(
		TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Hat_049.SK_Hat_049"));
	if (HeadMeshRef.Succeeded() == true)
	{
		HeadMesh->SetSkeletalMesh(HeadMeshRef.Object);
	}
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	HeadMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	HeadMesh->SetupAttachment(RootComponent);
	HeadMesh->SetLeaderPoseComponent(MeshComp);
	HeadMesh->SetReceivesDecals(false);

	FaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Face"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FaceMeshRef(
		TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Male_emotion_usual_001.SK_Male_emotion_usual_001"));
	if (FaceMeshRef.Succeeded() == true)
	{
		FaceMesh->SetSkeletalMesh(FaceMeshRef.Object);
	}
	FaceMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	FaceMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	FaceMesh->SetupAttachment(RootComponent);
	FaceMesh->SetLeaderPoseComponent(MeshComp);
	FaceMesh->SetReceivesDecals(false);

	TopMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Top"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> TopMeshRef(
		TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Outerwear_036.SK_Outerwear_036"));
	if (TopMeshRef.Succeeded() == true)
	{
		TopMesh->SetSkeletalMesh(TopMeshRef.Object);
	}
	TopMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	TopMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	TopMesh->SetupAttachment(RootComponent);
	TopMesh->SetLeaderPoseComponent(MeshComp);
	TopMesh->SetReceivesDecals(false);

	BottomMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Bottom"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BottomMeshRef(
		TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Pants_014.SK_Pants_014"));
	if (BottomMeshRef.Succeeded() == true)
	{
		BottomMesh->SetSkeletalMesh(BottomMeshRef.Object);
	}
	BottomMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	BottomMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	BottomMesh->SetupAttachment(RootComponent);
	BottomMesh->SetLeaderPoseComponent(MeshComp);
	BottomMesh->SetReceivesDecals(false);

	ShoesMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shoes"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ShoesMeshRef(
		TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Shoe_Slippers_005.SK_Shoe_Slippers_005"));
	if (ShoesMeshRef.Succeeded() == true)
	{
		ShoesMesh->SetSkeletalMesh(ShoesMeshRef.Object);
	}
	ShoesMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	ShoesMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	ShoesMesh->SetupAttachment(RootComponent);
	ShoesMesh->SetLeaderPoseComponent(MeshComp);
	ShoesMesh->SetReceivesDecals(false);

	FaceAccMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceAcc"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FaceAccMeshRef(
		TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Moustache_002.SK_Moustache_002"));
	if (FaceAccMeshRef.Succeeded() == true)
	{
		FaceAccMesh->SetSkeletalMesh(FaceAccMeshRef.Object);
	}
	FaceAccMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	FaceAccMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	FaceAccMesh->SetupAttachment(RootComponent);
	FaceAccMesh->SetLeaderPoseComponent(MeshComp);
	FaceAccMesh->SetReceivesDecals(false);

	//카메라
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetOffset = FVector(0.f, 60.f, 20.f); //솔더뷰, Y축으로 오른쪽으로 50

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	
	GunSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("GunSpringArm"));
	GunSpringArm->SetupAttachment(RootComponent);
	GunSpringArm->TargetArmLength = 0.0;
	GunSpringArm->bUsePawnControlRotation = true;
	GunSpringArm->SetRelativeLocation(FVector(0.f, 20.f, 40.f));
	
	GunCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("GunCamera"));
	GunCamera->SetupAttachment(GunSpringArm, USpringArmComponent::SocketName);
	GunCamera->bUsePawnControlRotation = false;
	
	InteractBoxComponent = CreateDefaultSubobject<UACInteractableComponent>(TEXT("InteractBoxComponent"));
	InteractBoxComponent->SetupAttachment(RootComponent);

	// 상호작용 위젯 컴포넌트
	InteractionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	InteractionWidgetComponent->SetupAttachment(RootComponent);
	InteractionWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionWidgetComponent->SetDrawSize(FVector2D(300.0f, 80.0f));
	InteractionWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // 머리 위
	InteractionWidgetComponent->SetVisibility(false); // 기본 숨김

	// 상호작용 위젯 클래스 설정
	static ConstructorHelpers::FClassFinder<UACInteractionInfoWidget> InteractionWidgetRef(
		TEXT("/Game/Project/UI/Interaction/WBP_InteractionInfo.WBP_InteractionInfo_C"));
	if (InteractionWidgetRef.Succeeded())
	{
		InteractionInfoWidgetClass = InteractionWidgetRef.Class;
	}

	// 애니메이션 몽타주
	static ConstructorHelpers::FObjectFinder<UAnimMontage> MeleeMontageRef(
		TEXT("/Game/Project/Character/AM_Melee.AM_Melee"));
	if (MeleeMontageRef.Succeeded())
	{
		MeleeMontage = MeleeMontageRef.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ZoomMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/Project/Character/AM_Zoom.AM_Zoom'"));
	if (ZoomMontageRef.Succeeded())
	{
		ZoomMontage = ZoomMontageRef.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ShootMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/Project/Character/AM_Shoot.AM_Shoot'"));
	if (ShootMontageRef.Succeeded())
	{
		ShootMontage = ShootMontageRef.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UAnimMontage> EscapeMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/Project/Character/AM_EscapeSkill.AM_EscapeSkill'"));
	if (EscapeMontageRef.Succeeded())
	{
		EscapeMontage = EscapeMontageRef.Object;
	}

	// ShopComponent 생성
	ShopComponent = CreateDefaultSubobject<UACShopComponent>(TEXT("ShopComponent"));
	GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
	MoneyComp = CreateDefaultSubobject<UACMoneyComponent>(TEXT("MoneyComponent"));


	bReplicates = true;

	bAlwaysRelevant = true;

	// 방망이 휘두르기 사운드 로드
	static ConstructorHelpers::FObjectFinder<USoundBase> BatSwingSoundRef(
		TEXT("/Game/Project/SFX/Attack/422506__nightflame__swinging-staff-whoosh-strong-01")
	);
	if (BatSwingSoundRef.Succeeded())
	{
		BatSwingSound = BatSwingSoundRef.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundBase> GunSoundRef(TEXT("/Script/Engine.SoundWave'/Game/Project/SFX/Gunshot/402010__eardeer__gunshot__high_1.402010__eardeer__gunshot__high_1'"));
	if (GunSoundRef.Succeeded())
	{
		GunSound = GunSoundRef.Object;
	}
	
	

	// VOIPTalker 생성
	VOIPTalker = CreateDefaultSubobject<UACVOIPTalker>(TEXT("VOIPTalker"));

	static ConstructorHelpers::FObjectFinder<USoundAttenuation> AttenuationProximityRef(TEXT("/Game/Project/Sound/SA_Proximity.SA_Proximity"));
	if (AttenuationProximityRef.Succeeded())
	{
		VoiceProximity = AttenuationProximityRef.Object;
	}
	static ConstructorHelpers::FObjectFinder<USoundAttenuation> AttenuationRadioRef(TEXT("/Game/Project/Sound/SA_Radio.SA_Radio"));
	if (AttenuationRadioRef.Succeeded())
	{
		VoiceRadio = AttenuationRadioRef.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UACSkillData> SkillDataAssetRef(TEXT("/Script/AnimalCrime.ACSkillData'/Game/Project/Character/DA_SkillData.DA_SkillData'"));
	if (SkillDataAssetRef.Succeeded())
	{
		SkillDataAsset = SkillDataAssetRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> StunOverlayRef(
		TEXT("/Game/Project/Character/Material/M_StunOverlay.M_StunOverlay")
	);
	if (StunOverlayRef.Succeeded())
	{
		StunOverlayMaterial = StunOverlayRef.Object;
	}
}

#pragma region 엔진 제공 함수

void AACCharacter::PostInitializeComponents()
{
	AC_LOG(LogHY, Error, TEXT("Begin"));
	Super::PostInitializeComponents();
	
	// 캐릭터 스킬
	SprintMoveSpeedData			= SkillDataAsset->SprintMoveSpeed;
	OriginMafiaMoveSpeedData	= SkillDataAsset->OriginMafiaMoveSpeed;
	OriginPoliceMoveSpeedData	= SkillDataAsset->OriginPoliceMoveSpeed;
	SprintGaugeData				= SkillDataAsset->SprintGauge;
	DashForwardImpulseData		= SkillDataAsset->DashForwardImpulse;
	DashUpwardImpulseData		= SkillDataAsset->DashUpwardImpulse;
	DashCoolTimeData			= SkillDataAsset->DashCoolTime;
	
	AC_LOG(LogHY, Error, TEXT("End"));
}

void AACCharacter::BeginPlay()
{
	AC_LOG(LogHY, Error, TEXT("Begin"));
	Super::BeginPlay();

	AC_LOG(LogHY, Error, TEXT("Before Speed %f %s"), GetCharacterMovement()->MaxWalkSpeed, *GetName());
	SetCharacterState(ECharacterState::Free);
	AC_LOG(LogHY, Error, TEXT("Before After %f %s"), GetCharacterMovement()->MaxWalkSpeed, *GetName());

	
	
	// @Todo 변경 필요. Mafia와 Police 구분이 안감.
	// Police와 Mafia는 각자의 BeginPlay에서 초기화
	// ※ 얘도 확인 했으면 지워주세요
	//MoneyComp->InitMoneyComponent(EMoneyType::MoneyMafiaType);

	// 로컬: Voice 시작
	if (IsLocallyControlled() == true)
	{
		// 로컬 플레이어: 딜레이 후 Voice 시작
		FTimerHandle VoiceStartTimer;
		GetWorldTimerManager().SetTimer(VoiceStartTimer, [this]()
			{
				UACAdvancedFriendsGameInstance* GI = GetGameInstance<UACAdvancedFriendsGameInstance>();
				if (GI == nullptr)
				{
					return;
				}
				GI->TryStartVoice();
			}, 0.5f, false);
		return;
	}
	// 원격 플레이어 VOIPTalker 등록
	// - 서버: OnRep 안 불리니까
	// - 클라이언트: 이미 PlayerState가 설정된 상태로 복제되면 OnRep 안 불리니까
	TryRegisterVOIPTalker();

	AC_LOG(LogHY, Error, TEXT("End"));
}

void AACCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	AC_LOG(LogVT, Log, TEXT("SYTEST - AACCharacter::OnRep_PlayerState %s"), *GetName());
	if (IsLocallyControlled() == true)
	{
		AC_LOG(LogVT, Log, TEXT("SYTEST - 로컬 플레이어이므로 VOIPTalker 등록 안함 %s"), *GetName());
		return;
	}
	AC_LOG(LogVT, Log, TEXT("SYTEST - 원격 플레이어이므로 VOIPTalker 등록 시도 %s"), *GetName());
	// 원격 플레이어: VOIPTalker 등록
	TryRegisterVOIPTalker();
}

void AACCharacter::TryRegisterVOIPTalker()
{
	if (VOIPTalker == nullptr)
	{
		AC_LOG(LogSY, Error, TEXT("VOIPTalker is nullptr in %s"), *GetName());
		return;
	}

	//서버의 경우 플레이어 스테이트가 바로 안생길수도 있어서 타이머로 재시도
	APlayerState* PS = GetPlayerState();
	if (PS == nullptr)
	{
		AC_LOG(LogSY, Log, TEXT("PlayerState is nullptr in %s, retrying..."), *GetName());
		// PlayerState가 아직 없으면 타이머로 재시도
		GetWorld()->GetTimerManager().SetTimer(
			VOIPTalkerTimerHandle,
			this,
			&AACCharacter::TryRegisterVOIPTalker,
			0.1f,
			false
		);
		return;
	}

	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (LocalPC == nullptr)
	{
		AC_LOG(LogVT, Warning, TEXT("Local PlayerController is nullptr in %s, retrying..."), *GetName());
		GetWorld()->GetTimerManager().SetTimer(
			VOIPTalkerTimerHandle,
			this,
			&AACCharacter::TryRegisterVOIPTalker,
			0.1f,
			false
		);
		return;
	}

	AACCharacter* LocalChar = Cast<AACCharacter>(LocalPC->GetPawn());
	if (LocalChar == nullptr)
	{
		AC_LOG(LogVT, Warning, TEXT("Local PlayerCharacter is nullptr in %s, retrying..."), *GetName());
		GetWorld()->GetTimerManager().SetTimer(
			VOIPTalkerTimerHandle,
			this,
			&AACCharacter::TryRegisterVOIPTalker,
			0.1f,
			false
		);
		return;
	}

	// 타이머 정리
	GetWorld()->GetTimerManager().ClearTimer(VOIPTalkerTimerHandle);

	if (VoiceProximity == nullptr)
	{
		AC_LOG(LogVT, Log, TEXT("VoiceProximity is nullptr in %s"), *GetName());
		return;
	}

	if (VoiceRadio == nullptr)
	{
		AC_LOG(LogVT, Log, TEXT("VoiceRadio is nullptr in %s"), *GetName());
		return;
	}

	// 음성이 캐릭터 위치에서 나도록 설정
	VOIPTalker->Settings.ComponentToAttachTo = GetRootComponent();

	// Register 전에 무전기 상태에 따라 Attenuation 결정
	bool bUseRadio = false;
	if (VoiceGroup != EVoiceGroup::None && VoiceGroup == LocalChar->VoiceGroup)
	{
		// 양쪽 다 무전기 있으면서 같은 무전기 그룹이면 Attenuation 제거
		bUseRadio = true;
		AC_LOG(LogVT, Log, TEXT("같은 무전기 그룹. VOIPTalker Radio mode for %s"), *GetName());
	}
	else
	{
		AC_LOG(LogVT, Log, TEXT("다른 무전기 그룹. VOIPTalker Proximity mode for %s"), *GetName());
	}

	VOIPTalker->Settings.AttenuationSettings = bUseRadio ? VoiceRadio : VoiceProximity;
	VOIPTalker->RegisterWithPlayerState(PS);

	AC_LOG(LogVT, Log, TEXT("VOIPTalker registered for %s, Attenuation: %s"), *GetName(), VOIPTalker->Settings.AttenuationSettings ? TEXT("Enabled") : TEXT("Disabled"));
	
	// VOIPTalker 설정 로드 (볼륨 크기)
	UACAdvancedFriendsGameInstance* GI = GetGameInstance<UACAdvancedFriendsGameInstance>();
	if (GI == nullptr)
	{
		UE_LOG(LogSY, Warning, TEXT("GameInstance is nullptr"));
		return;
	}
	VOIPTalker->SetVOIPVolume(GI->GetPlayerMicVolume(PS->GetPlayerName()));
	VOIPTalker->MuteToggle(GI->GetPlayerMicMute(PS->GetPlayerName()));
}

void AACCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 캐릭터 상태
	DOREPLIFETIME(AACCharacter, CharacterState);
	DOREPLIFETIME(AACCharacter, PrevCharacterState);

	// Skeletal Mesh
	DOREPLIFETIME(AACCharacter, HeadMeshReal);
	DOREPLIFETIME(AACCharacter, FaceMeshReal);
	DOREPLIFETIME(AACCharacter, TopMeshReal);
	DOREPLIFETIME(AACCharacter, BottomMeshReal);
	DOREPLIFETIME(AACCharacter, ShoesMeshReal);
	DOREPLIFETIME(AACCharacter, FaceAccMeshReal);

	// 나중에 총 객체 내부로 변경해야 함.
	DOREPLIFETIME(AACCharacter, BulletCount);
	//DOREPLIFETIME(AACCharacter, bHasRadio);
	DOREPLIFETIME(AACCharacter, VoiceGroup);

	DOREPLIFETIME(AACCharacter, bStun);
	DOREPLIFETIME(AACCharacter, bOnDamage);
	DOREPLIFETIME(AACCharacter, bInteract);

}

void AACCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AC_LOG(LogHY, Warning, TEXT("AACCharacter::EndPlay %s"), *GetName());

	// VOIPTalker 정리 (맵 이동 시 VoipListenerSynthComponent 정리를 위해)
	// DestroyComponent() 사용 금지 - 오디오 스레드가 비동기로 참조 중일 수 있음
	//if (VOIPTalker != nullptr)
	//{
	//	APlayerState* PS = GetPlayerState();
	//	if (PS != nullptr)
	//	{
	//		UVOIPStatics::ResetPlayerVoiceTalker(PS);
	//	}
	//	// DestroyComponent() 대신 Deactivate만 하고 GC에 맡김
	//	VOIPTalker->Deactivate();
	//	VOIPTalker = nullptr;
	//}

	// VOIPTalker 정리 (fallback). 맵 이동시에는 미리 정리함. 이건 강제 종료 시 정리용.
	CleanupVOIPTalker();

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

#pragma endregion

void AACCharacter::ChangeInputMode(EInputMode NewMode)
{
	AACPlayerControllerBase* PC = Cast<AACPlayerControllerBase>(GetController());  // Base로 변경
	if (PC)
	{
		PC->ChangeInputMode(NewMode);
	}
}

void AACCharacter::Move(const FInputActionValue& Value)
{
	if ((CharacterState == ECharacterState::Interact) || (CharacterState == ECharacterState::OnInteract))
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AACCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

// 기본값 0 (E 키)
void AACCharacter::InteractStarted(int32 InputIndex)
{
	if (bIsCarry == true)
	{
		AC_LOG(LogSY, Log, TEXT("Cannot interact while carrying"));
		return;
	}

	if (!FocusedInteractable || !IsValid(FocusedInteractable))
	{
		UpdateFocus();
		return;
	}

	IACInteractInterface* Interactable = Cast<IACInteractInterface>(FocusedInteractable);
	if (!Interactable)
		return;

	// PrisonDoor 예외: 문이 열려있으면 리턴
	if (Interactable->GetInteractorType() == EACInteractorType::PrisonDoor)
	{
		AACPrisonBase* Prison = Cast<AACPrisonBase>(FocusedInteractable);
		if (Prison && Prison->IsDoorOpen())
			return;
	}

	// InputIndex로 상호작용 선택 (E=0, R=1, T=2)
	if (!FocusedInteractions.IsValidIndex(InputIndex))
	{
		AC_LOG(LogSW, Warning, TEXT("No interaction at index %d"), InputIndex);
		return;
	}

	UACInteractionData* InteractionData = FocusedInteractions[InputIndex];
	if (!InteractionData)
		return;


	RequiredHoldTime = InteractionData->HoldDuration;
	const EInteractionKey Key = static_cast<EInteractionKey>(InputIndex);


	// 즉시 상호작용
	if (RequiredHoldTime <= KINDA_SMALL_NUMBER)
	{
		AC_LOG(LogSW, Warning, TEXT("Instant Interaction Started!!! %s | State: %s"), *GetName(), *UEnum::GetValueAsString(CharacterState));
		ServerInteract(FocusedInteractable, Key);
	}
	// 홀드 상호작용
	else
	{
		bIsHoldingInteract = true;
		CurrentHoldTarget = FocusedInteractable;
		CurrentHoldTime = 0.f;
		CurrentHoldKey = Key;
		AC_LOG(LogSW, Warning, TEXT("Hold Interaction Progress Started!!! %s | State: %s"), *GetName(), *UEnum::GetValueAsString(CharacterState));
		ServerFreezeCharacter(CurrentHoldTarget);

		// 서버에 홀드 상호작용 시작 알림 (몽타주 + 회전)
		ServerStartHoldInteraction(CurrentHoldTarget, InteractionData);

		AACMainPlayerController* PC = Cast<AACMainPlayerController>(GetController());
		if (PC)
			PC->ShowInteractProgress(InteractionData->InteractionName.ToString());
	}
}

void AACCharacter::InteractHolding(const float DeltaTime)
{
	//todo: bInteract로도 처리 가능한가?
	if (bIsHoldingInteract == false)
	{
		return;
	}

	if (CurrentHoldTarget == nullptr)
	{
		ResetHoldInteract();
		return;
	}
	if (IsValid(CurrentHoldTarget) == false)
	{
		ResetHoldInteract();
		return;
	}
	if (NearInteractables.Contains(CurrentHoldTarget) == false)
	{
		ResetHoldInteract();
		return;
	}


	if ((CharacterState == ECharacterState::OnDamage) || (CharacterState == ECharacterState::Stun))
	{
		AC_LOG(LogSW, Log, TEXT("%s's interaction was canceled!!"), *GetName())
			ResetHoldInteract();
		return;
	}

	// 시간 누적
	CurrentHoldTime += DeltaTime;

	// UI 업데이트
	AACMainPlayerController* PC = Cast<AACMainPlayerController>(GetController());
	if (PC)
	{
		PC->UpdateInteractProgress(GetHoldProgress());
	}

	// 홀드 시간 완료 시 상호작용 실행
	if (CurrentHoldTime >= RequiredHoldTime)
	{
		AC_LOG(LogSW, Warning, TEXT("Hold Interaction Progress Finished!!! %s | State: %s"), *GetName(), *UEnum::GetValueAsString(CharacterState));
		ServerInteract(CurrentHoldTarget, CurrentHoldKey);
		ResetHoldInteract();
	}
}

void AACCharacter::InteractReleased()
{
	ResetHoldInteract();
}

void AACCharacter::ItemDrop()
{
	UE_LOG(LogTemp, Log, TEXT("ItemDrop!!"));
}


void AACCharacter::Attack()
{
	if (CharacterState == ECharacterState::None || CharacterState == ECharacterState::Stun ||
		CharacterState == ECharacterState::Interact || CharacterState == ECharacterState::OnInteract)
	{
		return;
	}
	// 현재 공격 중인지 확인. 
	if (CheckProcessAttack() == true)
	{
		return;
	}

	if (bIsCarry == true)
	{
		AC_LOG(LogSY, Log, TEXT("Cannot interact while carrying"));
		return;
	}

	// Listen Server에 존재하는 클라이언트 경우
	if (HasAuthority())
	{
		PerformAttackTrace();
	}
	// Listen Server에 접속한 클라이언트 경우
	else
	{
		ServerAttack();
	}
}

void AACCharacter::SettingsClose()
{
	//자식에서 추가로 구현
	switch (SettingMode)
	{
	case ESettingMode::None:
		break;
	case ESettingMode::Default:
		break;
	case ESettingMode::SoundSetting:
		SetSoundSetting();
		break;
	default:
		break;
	}
}

void AACCharacter::SetSoundSetting()
{

	AACPlayerControllerBase* PC = Cast<AACPlayerControllerBase>(GetController());
	if (PC == nullptr)
	{
		return;
	}

	//설정창이 꺼져있으면 소리 설정창 오픈, 소리 설정창이 켜져있으면 끄기, 다른 설정창이면 아무것도 안함.
	if (SettingMode == ESettingMode::None)
	{
		PC->SoundSettingToggle(true);
		ChangeInputMode(EInputMode::Settings);
		SettingMode = ESettingMode::SoundSetting;
	}
	else if (SettingMode == ESettingMode::SoundSetting)
	{
		PC->SoundSettingToggle(false);
		ChangeInputMode(EInputMode::Sholder);
		SettingMode = ESettingMode::None;
	}
}

#pragma region 캐릭터 스킬 - Dash(1)

void AACCharacter::Dash(const FInputActionValue& Value)
{
	bool bDashFlag = Value.Get<bool>();
	AC_LOG(LogHY, Log, TEXT("PlayerController -> AACCharacter | Input: %d"), bDashFlag);

	// Case: 심각한 에러 - 나와서는 안되는 값.
	if (bDashFlag == false)
	{
		AC_LOG(LogHY, Error, TEXT("Dash Input False"));
		return;
	}

	if (bIsCarry == true)
	{
		AC_LOG(LogSY, Log, TEXT("Cannot interact while carrying"));
		return;
	}

	// Server에게 Dash Skill 사용 요청
	ServerDash();
}

void AACCharacter::ServerDash_Implementation()
{
	// DashTimerHandle이 살아있는 경우는 실행 불가.
	if (GetWorldTimerManager().IsTimerActive(DashTimerHandle) == true)
	{
		AC_LOG(LogHY, Error, TEXT("Dash Timer is already active"));
		return;
	}

	bDashCoolDown = false;

	AC_LOG(LogHY, Error, TEXT("Excute [Server Dash]"));

	// 캐릭터가 바라보는 방향 획득
	FVector ForwardDir = GetActorForwardVector();

	// 날아가는 힘 계산
	FVector LaunchVelocity = ForwardDir * DashForwardImpulseData + FVector(0.f, 0.f, DashUpwardImpulseData);
	LaunchCharacter(LaunchVelocity, true, false);

	// Dash 사용 쿨타임 적용.
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AACCharacter::ResetDashFlag);
	GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, TimerDelegate, DashCoolTimeData, false);
}

void AACCharacter::ResetDashFlag()
{
	bDashCoolDown = true;
}

#pragma endregion

#pragma region 캐릭터 스킬 - Sprint(2)

void AACCharacter::Sprint(const FInputActionValue& Value)
{
	if (bIsCarry == true)
	{
		AC_LOG(LogSY, Log, TEXT("Cannot interact while carrying"));
		return;
	}
	if (Value.Get<bool>())
	{
		if (SprintGauge > 0)
		{
			ServerSprintStart();
		}
	}
	else
	{
		ServerSprintEnd();
	}
}

AACMainPlayerController* AACCharacter::GetMainPlayerController() const
{
	return Cast<AACMainPlayerController>(GetController());
}

void AACCharacter::ServerSprintStart_Implementation()
{
	if (bSprint == true)
	{
		AC_LOG(LogHY, Error, TEXT("Sprint Timer is already active"));
		return;
	}

	// Delete Guage Up 
	GetWorld()->GetTimerManager().ClearTimer(SprintGaugeUpTimerHandle);

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AACCharacter::GaugeDown);

	GetWorld()->GetTimerManager().SetTimer(SprintGaugeDownTimerHandle, TimerDelegate, 1, true);

	bSprint = true;
	OnRep_Sprint();
}

void AACCharacter::ServerSprintEnd_Implementation()
{
	if (bSprint == false)
	{
		AC_LOG(LogHY, Error, TEXT("Sprint Timer is already deactive"));
		// return은 필요 없음.
	}

	// 게이지 Down에 대한 Timer 종료
	GetWorld()->GetTimerManager().ClearTimer(SprintGaugeDownTimerHandle);

	// 게이지 Up에 대한 Timer 시작
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AACCharacter::GaugeUp);
	if (GetWorld()->GetTimerManager().IsTimerActive(SprintGaugeUpTimerHandle) == false)
	{
		GetWorld()->GetTimerManager().SetTimer(SprintGaugeUpTimerHandle, TimerDelegate, 1, true);
	}

	bSprint = false;
	OnRep_Sprint();
}

void AACCharacter::OnRep_Sprint()
{
	GetCharacterMovement()->MaxWalkSpeed = CalculateMoveSpeed();
	// if (bSprint)
	// {
	// 	//GetCharacterMovement()->MaxWalkSpeed = SprintMoveSpeedData;
	// 	
	// 	// AC_LOG(LogHY, Warning, TEXT("Sprint is active %f"), GetCharacterMovement()->MaxWalkSpeed);
	// }
	// else
	// {
	// 	if (GetCharacterType() == EACCharacterType::Mafia)
	// 	{
	// 		GetCharacterMovement()->MaxWalkSpeed = OriginMafiaMoveSpeedData;
	// 	}
	// 	else if (GetCharacterType() == EACCharacterType::Police)
	// 	{
	// 		GetCharacterMovement()->MaxWalkSpeed = OriginPoliceMoveSpeedData;
	// 	}
	// }
}

void AACCharacter::GaugeUp()
{
	// 1씩 증가
	if (SprintGauge + 1 > SprintGaugeData)
	{
		GetWorldTimerManager().ClearTimer(SprintGaugeUpTimerHandle);
		bSprint = false;
		OnRep_Sprint();
		return;
	}
	// AC_LOG(LogHY, Warning, TEXT("Gauge Up: %d"), SprintGauge);
	
	SprintGauge += 1;
	OnRep_SprintGauge();
}

void AACCharacter::GaugeDown()
{
	// AC_LOG(LogHY, Warning, TEXT("Gauge Down: %d"), SprintGauge);
	if (SprintGauge - 1 <= 0)
	{
		GetWorldTimerManager().ClearTimer(SprintGaugeDownTimerHandle);
		
		// @Todo: 임시 방편 바꿔야할수도...
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &AACCharacter::GaugeUp);
		if (GetWorld()->GetTimerManager().IsTimerActive(SprintGaugeUpTimerHandle) == false)
		{
			GetWorld()->GetTimerManager().SetTimer(SprintGaugeUpTimerHandle, TimerDelegate, 1, true);
		}
		bSprint = false;
		OnRep_Sprint();
	}
	
	SprintGauge -= 1;
	OnRep_SprintGauge();
}

#pragma endregion

void AACCharacter::Jump()
{
	// Case: 스턴 및 감옥 상태일 경우 Jump 불가 
	if (CharacterState == ECharacterState::None ||
		CharacterState == ECharacterState::Stun ||
		CharacterState == ECharacterState::OnInteract ||
		CharacterState == ECharacterState::Interact ||
		CharacterState == ECharacterState::Prison)
	{
		return;
	}

	// Case 감옥 상태일 경우 
	if (CharacterState == ECharacterState::OnInteract)
	{
		return;
	}

	if (bIsCarry == true)
	{
		AC_LOG(LogSY, Log, TEXT("Cannot interact while carrying"));
		return;
	}

	Super::Jump();
}

void AACCharacter::ServerInteract_Implementation(AActor* Target, EInteractionKey InKey)
{
	if (Target == nullptr)
	{
		AC_LOG(LogSW, Log, TEXT("NULL!!"));
		return;
	}

	if (IsValid(Target) == false)
	{
		AC_LOG(LogSW, Log, TEXT("Interactable Deleted!!"));
		return;
	}

	IACInteractInterface* Interactable = Cast<IACInteractInterface>(Target);
	if (Interactable == nullptr)
	{
		AC_LOG(LogSW, Log, TEXT("%s has No Interface!!"), *Target->GetName());
		return;
	}


	Interactable->OnInteract(this, InKey);
	AC_LOG(LogSW, Log, TEXT("%s Interacted with %s!!"), *Target->GetName(), *GetName());
}

bool AACCharacter::CheckProcessAttack() const
{
	return bAttackFlag;
}

void AACCharacter::ChangeAttackTrue()
{
	bAttackFlag = true;
}

void AACCharacter::ChangeAttackFalse()
{
	bAttackFlag = false;
}

void AACCharacter::PerformAttackTrace()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return;
	}

	// Todo: 멀티캐스트에서 변경하기
	MulticastPlayAttackMontage();
}

// void AACCharacter::ServerEscape_Implementation()
// {
// 	PerformEscape();
// 	
// }
//
// void AACCharacter::PerformEscape()
// {
// 	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
// 	if (AnimInstance == nullptr)
// 	{
// 		return;
// 	}
//
// 	
// 	MulticastPlayEscapeSkillMontage();
// 	
// }

void AACCharacter::AttackHitCheck(int32 DamageAmount)
{
	// Empty Class
	// 자식 클래스에서 구현
}

bool AACCharacter::IsHoldingGun()
{
	UACItemData* EquippedWeapon = ShopComponent->EquippedWeapon;
	if (EquippedWeapon == nullptr)
	{
		AC_LOG(LogHY, Log, TEXT("EquippedWeapon is nullptr"));
		return false;
	}
	if (EquippedWeapon->ItemType != EItemType::Weapon)
	{
		AC_LOG(LogTemp, Log, TEXT(">>> Weapon Equipped: %s"), *EquippedWeapon->ItemName.ToString());
		return false;
	}
	if (EquippedWeapon->ItemName.ToString() != TEXT("Pistol_001"))
	{
		AC_LOG(LogTemp, Log, TEXT(">>>	 Weapon Equipped: %s"), *EquippedWeapon->ItemName.ToString());
		return false;
	}
	return true;
}

void AACCharacter::MulticastStopAllMontage_Implementation()
{
	AC_LOG(LogHY, Error, TEXT("MulticastStopAllMontage Before"));
	
	if (ZoomMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		AC_LOG(LogHY, Error, TEXT("MulticastStopAllMontage After"));
		GetMesh()->GetAnimInstance()->StopAllMontages(0.5);
	}
}

void AACCharacter::MulticastPlayZoomMontage_Implementation()
{
	AC_LOG(LogHY, Error, TEXT("MulticastPlayZoomMontage Before"));
	
	if (ZoomMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		AC_LOG(LogHY, Error, TEXT("MulticastPlayZoomMontage After"));
		GetMesh()->GetAnimInstance()->Montage_Play(ZoomMontage, 1.0f);
	}
}

void AACCharacter::MulticastPlayShootMontage_Implementation()
{
	// 몽타주 재생.
	if (ShootMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		// Notify 이용중임.
		GetMesh()->GetAnimInstance()->Montage_Play(ShootMontage, 1.0f);
	}

	// 로컬 클라이언트 + 무기 장착 시에만 사운드 재생
	if (GunSound && ShopComponent)
	{
		UACItemData* EquippedWeapon = ShopComponent->EquippedWeapon;

		// 무기를 들고 있는지 확인
		if (EquippedWeapon != nullptr && EquippedWeapon->ItemType == EItemType::Weapon)
		{
			UGameplayStatics::PlaySound2D(this, GunSound);
		}
	}
}

// AnimNotify에서 실행하는 함수.
void AACCharacter::FireHitscan()
{
	if (IsHoldingGun() == false)
	{
		return;
	}

	if (GetBulletCount() <= 0)
	{
		AC_LOG(LogTemp, Log, TEXT("총알 갯수 %d"), GetBulletCount());
		return;
	}

	if (bIsCarry == true)
	{
		AC_LOG(LogSY, Log, TEXT("Cannot interact while carrying"));
		return;
	}
	
	// if (GetMesh()->GetAnimInstance()->Montage_IsActive(ShootMontage) == true)
	// {
	// 	AC_LOG(LogHY, Log, TEXT("몽타주가 재생 중 입니다."));
	// 	return;
	// }

	ServerShoot();
}

void AACCharacter::SetCarryState(bool bPlay)
{
	if (HasAuthority())
	{
		Multicast_SetCarryState(bPlay);
	}
	else
	{
		Server_SetCarryState(bPlay);
	}
}

void AACCharacter::Server_SetCarryState_Implementation(bool bPlay)
{
	Multicast_SetCarryState(bPlay);
}

void AACCharacter::Multicast_SetCarryState_Implementation(bool bPlay)
{
	UACCharacterAnimInstance* Anim = Cast<UACCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (Anim == nullptr)
	{
		return;
	}
	bIsCarry = bPlay;
	Anim->SetIsCarrying(bPlay);
}

void AACCharacter::OnInteract(AACCharacter* ACPlayer, EInteractionKey InKey)
{
}

EACInteractorType AACCharacter::GetInteractorType() const
{
	return EACInteractorType::Citizen;
}

UWidgetComponent* AACCharacter::GetInteractionWidget() const
{
	return InteractionWidgetComponent;
}

void AACCharacter::ShowInteractionHints(const TArray<UACInteractionData*>& Interactions)
{
	if (!InteractionWidgetComponent) return;

	if (IsValid(InteractionWidgetComponent) == false)
	{
		return;
	}

	// 위젯 생성 (최초 1회)
	if (!InteractionWidgetComponent->GetWidget() && InteractionInfoWidgetClass)
	{
		InteractionWidgetComponent->SetWidgetClass(InteractionInfoWidgetClass);
	}

	UACInteractionInfoWidget* HintWidget = Cast<UACInteractionInfoWidget>(
		InteractionWidgetComponent->GetWidget());
	if (!HintWidget) return;

	HintWidget->UpdateInteractions(Interactions);
	HintWidget->ShowWidget();
	InteractionWidgetComponent->SetVisibility(true);
}

void AACCharacter::HideInteractionHints()
{
	if (!InteractionWidgetComponent) return;

	if (IsValid(InteractionWidgetComponent) == false)
	{
		return;
	}

	UACInteractionInfoWidget* HintWidget = Cast<UACInteractionInfoWidget>(
		InteractionWidgetComponent->GetWidget());
	if (HintWidget)
		HintWidget->HideWidget();

	InteractionWidgetComponent->SetVisibility(false);
}

void AACCharacter::UpdateFocus()
{
	// 로컬 플레이어만 상호작용 UI 표시
	if (!IsLocallyControlled())
	{
		return;
	}

	// 홀드 상호작용 중에는 포커스 갱신을 막음 (상태 변경으로 인해 UI가 사라지는 것 방지)
	if (bIsHoldingInteract)
	{
		return;
	}

	AActor* PreviousFocus = FocusedInteractable;
	FocusedInteractable = nullptr;
	IACInteractInterface* PrevInteractable = nullptr;

	if (PreviousFocus != nullptr && IsValid(PreviousFocus))
	{
		PrevInteractable = Cast<IACInteractInterface>(PreviousFocus);
		if (PrevInteractable == nullptr)
		{
			return;
		}
	}

	// 1. 거리순 정렬
	if (SortNearInteractables() == false)
	{
		// 주변에 상호작용 가능한 객체 없음 - 이전 Focus 위젯 숨김
		if (PreviousFocus != nullptr && IsValid(PreviousFocus))
		{
			PrevInteractable->HideInteractionHints();
		}
		return;
	}

	// PlayerState의 Location을 Prison으로 상태 변경


	// 2. 첫 번째 유효한 액터 찾기
	UACInteractionSubsystem* InteractionSys = GetGameInstance()->GetSubsystem<UACInteractionSubsystem>();
	if (InteractionSys == nullptr || IsValid(InteractionSys) == false)
	{
		return;
	}
	UACInteractionDatabase* DB = InteractionSys->GetInteractionDatabase();
	if (DB == nullptr)
	{
		return;
	}

	AACPlayerState* PS = GetPlayerState<AACPlayerState>();
	if (PS == nullptr)
	{
		return;
	}

	EACCharacterType CharacterType = GetCharacterType();
	IACInteractInterface* FocusedInterface = nullptr;
	EACInteractorType FocusedTargetType = EACInteractorType::Citizen;

	for (AActor* Candidate : NearInteractables)
	{
		if (!IsValid(Candidate)) continue;

		IACInteractInterface* Interactable = Cast<IACInteractInterface>(Candidate);
		if (!Interactable) continue;

		EACInteractorType TargetType = Interactable->GetInteractorType();

		// DB에 상호작용이 없으면 스킵
		if (DB->HasInteractions(CharacterType, TargetType) == false)
			continue;

		// PrisonDoor 예외: 문이 열려있으면 스킵
		if (TargetType == EACInteractorType::PrisonDoor)
		{
			AACPrisonBase* Prison = Cast<AACPrisonBase>(Candidate);
			if (Prison && Prison->IsDoorOpen())
				continue;
		}

		// todo: 임시로 감옥 안의 마피아는 건들지 말자
		if (CharacterType == EACCharacterType::Police && TargetType == EACInteractorType::Mafia)
		{
			AACCharacter* TargetChar = Cast<AACCharacter>(Candidate);
			if (TargetChar == nullptr)
			{
				AC_LOG(LogSW, Error, TEXT("NOT AACCharacter"))
					continue;
			}
			AACPlayerState* mafPS = TargetChar->GetPlayerState<AACPlayerState>();
			if (mafPS == nullptr)
			{
				AC_LOG(LogSW, Error, TEXT("NO GetPlayerState"))
					continue;
			}

			if (mafPS->CharacterLocation == ECharacterLocation::Prison)
			{
				AC_LOG(LogSW, Error, TEXT("STOP PLAYING WITH PRISONERS!!!!"))
					continue;
			}
		}

		FocusedInteractable = Candidate;
		FocusedInterface = Interactable;
		FocusedTargetType = TargetType;
		break;
	}

	// 3. Focus 변경 시 이전 위젯 숨김
	if (FocusedInteractable != PreviousFocus)
	{
		// 이전 Focus 위젯 숨김
		if (PreviousFocus != nullptr && IsValid(PreviousFocus))
		{
			PrevInteractable->HideInteractionHints();
		}
	}

	// 4. 현재 Focus 위젯 업데이트 (Focus가 같아도 상태 변경으로 인해 목록이 바뀔 수 있음)
	if (FocusedInteractable && FocusedInterface && DB)
	{
		// 하나라도 표기 가능하면 true설정해준다.
		bool bHasUsefulInteraction = false;

		// FocusedInteractions 초기화 (최대 키 개수만큼, nullptr로 채움)
		const int32 MaxKeys = static_cast<int32>(EInteractionKey::Total);
		FocusedInteractions.Init(nullptr, MaxKeys);

		TArray<UACInteractionData*> AllInteractions = DB->GetInteractions(CharacterType, FocusedTargetType);

		for (UACInteractionData* InteractionData : AllInteractions)
		{
			if (!InteractionData) continue;

			// 1) 주체(나) 상태 체크
			if (InteractionData->InteractorState.IsEmpty() == false)
			{
				if (InteractionData->InteractorState.Contains(CharacterState) == false)
				{
					// 내 상태가 조건에 없다면 스킵
					continue;
				}
			}

			// 2) 주체(나) 위치 체크
			if (InteractionData->InteractorLocation != PS->CharacterLocation)
			{
				AC_LOG(LogSW, Error, TEXT("%d is not %d Location!!!"), PS->CharacterLocation, InteractionData->InteractorLocation)
					continue;
			}

			// 3) 대상 상태 체크 (대상이 캐릭터인 경우만)
			if (InteractionData->bIsCharacterInteraction == true)
			{
				if (InteractionData->TargetState.IsEmpty() == false)
				{
					AACCharacter* TargetChar = Cast<AACCharacter>(FocusedInteractable);
					if (TargetChar != nullptr)
					{
						if (InteractionData->TargetState.Contains(TargetChar->CharacterState) == false)
						{
							// 대상 상태가 조건에 없다면 스킵
							continue;
						}
					}
				}
			}

			bHasUsefulInteraction = true;
			// 키 매핑 (Enum 값을 바로 인덱스로 사용)
			int32 KeyIndex = static_cast<int32>(InteractionData->AssignedKey);

			if (KeyIndex >= 0 && KeyIndex < MaxKeys)
			{
				FocusedInteractions[KeyIndex] = InteractionData;
			}
		}

		if (bHasUsefulInteraction == false)
		{
			FocusedInterface->HideInteractionHints();
		}
		else
		{
			FocusedInterface->ShowInteractionHints(FocusedInteractions);
		}
	}

}

void AACCharacter::AddInteractable(AActor* Interactor)
{
	//ensureAlways(Interactor);
	NearInteractables.AddUnique(Interactor);
	UpdateFocus();
}

void AACCharacter::RemoveInteractable(AActor* Interactor)
{
	//ensureAlways(Interactor);
	NearInteractables.Remove(Interactor);
	UpdateFocus();
}

bool AACCharacter::SortNearInteractables()
{
	for (int32 i = NearInteractables.Num() - 1; i >= 0; --i)
	{
		if (IsValid(NearInteractables[i]) == true)
		{
			continue;
		}

		NearInteractables.RemoveAt(i);
	}

	if (NearInteractables.Num() == 0)
	{
		AC_LOG(LogSW, Log, TEXT("No Close Interactables!!"));
		return false;
	}

	if (NearInteractables.Num() == 1)
	{
		//AC_LOG(LogSW, Log, TEXT("NO NEED TO SORT!!"));
		return true;
	}

	const FVector PlayerLocation = GetActorLocation();
	NearInteractables.Sort([PlayerLocation](const AActor& A, const AActor& B)
		{
			float DistA = FVector::DistSquared(PlayerLocation, A.GetActorLocation());
			float DistB = FVector::DistSquared(PlayerLocation, B.GetActorLocation());
			return DistA < DistB;
		});

	return true;
}

void AACCharacter::OnRep_CharacterState()
{
	if (IsValid(this) == false)
	{
		AC_LOG(LogHY, Error, TEXT("this is Invalid"));
		return;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("MoveComp is nullptr"));
		return;
	}

	AC_LOG(LogSW, Error, TEXT("CharacterType:%d name:%s"), CharacterState, *GetName());

	switch (CharacterState)
	{
	case ECharacterState::Free:
	{
		SetFreeState();
		break;
	}
	case ECharacterState::Stun:
	{
		SetStunState();
		break;
	}
	case ECharacterState::OnDamage:
	{
		SetOnDamageState();
		break;
	}
	case ECharacterState::Interact:
	{
		SetInteractState();
		break;
	}
	case ECharacterState::OnInteract:
	{
		SetOnInteractState();
		break;
	}
	}

	// todo: 시민은 처리 안해주고있음
	//// Focus된 캐릭터의 상태가 변경되면 상호작용 재계산
	if (GetWorld())
	{
		// 1. 내 상태가 변경된 경우 (내가 로컬 플레이어)
		UpdateFocus();

		// 2. 타인의 상태가 변경된 경우 (로컬 플레이어가 나를 보고 있는지 확인)
		APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
		if (LocalPC && LocalPC->IsLocalController())
		{
			AACCharacter* LocalChar = Cast<AACCharacter>(LocalPC->GetPawn());
			if (LocalChar && LocalChar != this && LocalChar->FocusedInteractable == this)
			{
				// 로컬 플레이어가 나를 보고 있다면, 로컬 플레이어의 UI 갱신
				LocalChar->UpdateFocus();
			}
		}
	}
}



ECharacterState AACCharacter::GetCharacterState() const
{
	return CharacterState;
}

ECharacterState AACCharacter::GetPrevCharacterState() const
{
	return PrevCharacterState;
}

void AACCharacter::SetCharacterState(ECharacterState InCharacterState)
{
	// Case: 상태각 같은 경우
	if (CharacterState == InCharacterState)
	{
		AC_LOG(LogHY, Warning, TEXT("Fail Same Now: %s Input: %s"), *UEnum::GetValueAsString(CharacterState), *UEnum::GetValueAsString(InCharacterState));
		return;
	}

	// Case: Free에서 갈 수 있는 케이스 여부
	// Free->None (X)
	// Free->OnDamage
	// Free->Interact
	// Free->OnInteract
	// Free->Stun
	if (CharacterState == ECharacterState::Free)
	{
		// Free로만 변경가능.	보류( || (InCharacterState == ECharacterState::Prison))

		if ((InCharacterState == ECharacterState::None))
		{
			AC_LOG(LogHY, Warning, TEXT("Fail Now: %s Input: %s name:%s"), *UEnum::GetValueAsString(CharacterState), *UEnum::GetValueAsString(InCharacterState), *GetName());
			return;
		}
		if ((InCharacterState == ECharacterState::Interact))
		{
			bInteract = true;
		}
		if ((InCharacterState == ECharacterState::OnInteract))
		{
			bOnInteract = true;
		}
	}

	// Case: Free에서 갈 수 있는 케이스 여부
	// OnDamage->None (X)
	// OnDamage->Free
	// OnDamage->OnDamage (X)
	// OnDamage->Interact (X)
	// OnDamage->OnInteract (X)
	// OnDamage->Stun (X)
	if (CharacterState == ECharacterState::OnDamage)
	{
		// Free로만 변경가능.
		if ((InCharacterState == ECharacterState::None) || (InCharacterState == ECharacterState::Interact) ||
			(InCharacterState == ECharacterState::OnInteract) || (InCharacterState == ECharacterState::Angry) ||
			(InCharacterState == ECharacterState::Stun))
		{
			AC_LOG(LogHY, Warning, TEXT("Fail Now: %s Input: %s name:%s"), *UEnum::GetValueAsString(CharacterState), *UEnum::GetValueAsString(InCharacterState), *GetName());
			return;
		}
	}

	// Interact->None (X),
	// Interact->Free (X)
	// Interact->OnDamage
	// Interact->Interact (X),
	// Interact->OnInteract (X)
	// Interact->Stun 
	if (CharacterState == ECharacterState::Interact)
	{
		if ((InCharacterState == ECharacterState::None) || (InCharacterState == ECharacterState::OnInteract) ||
			(InCharacterState == ECharacterState::Angry))
		{
			AC_LOG(LogHY, Warning, TEXT("Fail Now: %s Input: %s name:%s"), *UEnum::GetValueAsString(CharacterState), *UEnum::GetValueAsString(InCharacterState), *GetName());
			return;
		}
		bInteract = false;
	}

	// OnInteract->None (X),
	// OnInteract->Free
	// OnInteract->OnDamage (X)
	// OnInteract->Interact (X),
	// OnInteract->OnInteract (X)
	// OnInteract->Stun (X)
	if (CharacterState == ECharacterState::OnInteract)
	{
		if ((InCharacterState == ECharacterState::None) || (InCharacterState == ECharacterState::OnDamage) || (InCharacterState == ECharacterState::Interact) ||
			(InCharacterState == ECharacterState::Stun))
		{
			AC_LOG(LogHY, Warning, TEXT("Fail Now: %s Input: %s name:%s"), *UEnum::GetValueAsString(CharacterState), *UEnum::GetValueAsString(InCharacterState), *GetName());
			return;
		}
		bOnInteract = false;
	}

	// Stun->None (X),
	// Stun->Free
	// Stun->OnDamage (X)
	// Stun->Interact (X),
	// Stun->OnInteract
	// Stun->Stun (X)
	if (CharacterState == ECharacterState::Stun)
	{
		if ((InCharacterState == ECharacterState::None) || (InCharacterState == ECharacterState::OnDamage) ||
			(InCharacterState == ECharacterState::Interact))
		{
			AC_LOG(LogHY, Warning, TEXT("Fail Now: %s Input: %s name:%s"), *UEnum::GetValueAsString(CharacterState), *UEnum::GetValueAsString(InCharacterState), *GetName());
			return;
		}

		if ((InCharacterState == ECharacterState::OnInteract))
		{
			bOnInteract = true;
		}
	}

	AC_LOG(LogHY, Warning, TEXT("Success !!! Now: %s Input: %s name:%s"), *UEnum::GetValueAsString(CharacterState), *UEnum::GetValueAsString(InCharacterState), *GetName());

	PrevCharacterState = CharacterState;
	CharacterState = InCharacterState;

	if (HasAuthority())
	{
		OnRep_CharacterState();
	}
}

void AACCharacter::SetFreeState()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("MoveComp is nullptr"));
		return;
	}

	// Case: Police
	// if (GetCharacterType() == EACCharacterType::Police)
	// {
	// 	MoveComp->MaxWalkSpeed = OriginPoliceMoveSpeedData;; 
	// 		
	// }
	// // Case: Mafia
	// else
	// {
	// 	MoveComp->MaxWalkSpeed = OriginMafiaMoveSpeedData;
	// }

	// 테스트
	bOnDamage = false;
	bStun = false;

	RemoveStunOverlay();

	AC_LOG(LogHY, Error, TEXT("Before speed: %f"), MoveComp->MaxWalkSpeed);
	MoveComp->MaxWalkSpeed = CalculateMoveSpeed();
	AC_LOG(LogHY, Error, TEXT("After  speed: %f"), MoveComp->MaxWalkSpeed);
	MoveComp->JumpZVelocity = OriginZVelocity;
}

void AACCharacter::SetOnDamageState()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("MoveComp is nullptr"));
		return;
	}

	bOnDamage = true;
	AC_LOG(LogHY, Error, TEXT("Before speed: %f"), MoveComp->MaxWalkSpeed);
	MoveComp->MaxWalkSpeed = CalculateMoveSpeed();
	AC_LOG(LogHY, Error, TEXT("After  speed: %f"), MoveComp->MaxWalkSpeed);
}

void AACCharacter::SetStunState()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("MoveComp is nullptr"));
		return;
	}

	// 피격 효과 타이머 클리어
	// 스턴 상태로 전환될 때 기존 피격 타이머를 제거하여
	// ResetHitEffect()가 호출되지 않도록 함
	if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(HitEffectTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(HitEffectTimerHandle);
		AC_LOG(LogHG, Warning, TEXT("Cleared HitEffect timer for Stun"));
	}

	// MovementComponent 속성 변경
	bStun = true;

	ApplyStunOverlay();

	AC_LOG(LogHY, Error, TEXT("Before speed: %f"), MoveComp->MaxWalkSpeed);
	MoveComp->MaxWalkSpeed = CalculateMoveSpeed();
	AC_LOG(LogHY, Error, TEXT("After  speed: %f"), MoveComp->MaxWalkSpeed);
	MoveComp->JumpZVelocity = StunZVelocity;

	// 컨트롤러의 HUD 변경
	// AACMainPlayerController* MainPlayerController = GetMainPlayerController();
	// if (MainPlayerController == nullptr)
	// {
	// 	AC_LOG(LogHY, Error, TEXT("MainPlayerController is nullptr"));
	// 	return;
	// }
	// MainPlayerController->ZoomOut();
}

//void AACCharacter::SetPrisonState()
//{
//	Stat->SetCurrentHp(Stat->GetMaxHp());	
//}
//
//void AACCharacter::SetPrisonEscapeState()
//{
//	// Todo: 언젠가 Free로 되돌려야 함.
//	
//	// FTimerDelegate TimerDelegate;
//	// TimerDelegate.BindUObject(this, &AACCharacter::SetPrisonEscapeState);
//	// GetWorld()->GetTimerManager().SetTimer(EscapeTimerHandle, TimerDelegate, 3, false);
//}

void AACCharacter::SetInteractState()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("MoveComp is nullptr"));
		return;
	}

	AC_LOG(LogSW, Error, TEXT("%s changed to Interact"), *GetName());
	MoveComp->MaxWalkSpeed = CalculateMoveSpeed();
}

void AACCharacter::SetOnInteractState()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("MoveComp is nullptr"));
		return;
	}

	AC_LOG(LogSW, Error, TEXT("%s changed to OnInteract"), *GetName());
	MoveComp->MaxWalkSpeed = CalculateMoveSpeed();
}

float AACCharacter::CalculateMoveSpeed() const
{
	float Speed = 600.0f;

	// AC_LOG(LogHY, Error, TEXT("Type: %s"), *StaticEnum<EACCharacterType>()->GetNameStringByValue(static_cast<int64>(GetCharacterType())));
	// Free 상태일 때
	if (GetCharacterType() == EACCharacterType::Police)
	{
		Speed = OriginPoliceMoveSpeedData;
	}
	// Case: Mafia
	else if (GetCharacterType() == EACCharacterType::Mafia)
	{
		Speed = OriginMafiaMoveSpeedData;
	}

	if (bOnDamage)
	{
		if (GetCharacterType() == EACCharacterType::Police)
		{
			Speed -= 200;
		}
		else if (GetCharacterType() == EACCharacterType::Mafia)
		{
			Speed += 200;
		}
		else
		{
			AC_LOG(LogHY, Error, TEXT("과연 뭘까?"));
		}
	}

	// @Todo 애매...
	Speed += SprintMoveSpeedData;
	if (bSprint)
	{
		Speed += SprintMoveSpeedData;
	}
	else
	{
		Speed -= SprintMoveSpeedData;
	}

	if (bStun)
	{
		Speed = StunWalkSpeed;
	}

	if (bOnInteract || bInteract)
	{
		AC_LOG(LogHY, Error, TEXT("bOnInteract:%d, bInteract:%d"), bOnInteract, bInteract);
		Speed = 0;
	}

	return Speed;
}

void AACCharacter::ResetHoldInteract()
{
	if (!bIsHoldingInteract)
	{
		return;
	}

	// 로컬 UI 정리
	AACMainPlayerController* PC = Cast<AACMainPlayerController>(GetController());
	if (PC)
	{
		PC->HideInteractProgress();
	}

	// 타겟이 있으면 정리
	if (CurrentHoldTarget != nullptr)
	{
		// 상태 초기화
		ServerUnfreezeCharacter(CurrentHoldTarget);

		// 몽타주 초기화
		ServerStopHoldInteraction(CurrentHoldTarget);

		// 즉시 상호작용 재시도 방지
		RemoveInteractable(CurrentHoldTarget);
	}

	bIsHoldingInteract = false;
	CurrentHoldTarget = nullptr;
	CurrentHoldTime = 0.f;
	RequiredHoldTime = 0.f;
	//bOnInteract = false;

	UpdateFocus();
}

float AACCharacter::GetHoldProgress() const
{
	if (RequiredHoldTime <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(CurrentHoldTime / RequiredHoldTime, 0.f, 1.f);
}

void AACCharacter::CleanupVOIPTalker()
{
	if (VOIPTalker == nullptr)
	{
		return;  // 이미 정리됨
	}

	AC_LOG(LogSY, Log, TEXT("CleanupVOIPTalker - Character: %s"), *GetName());

	// PlayerState를 통해 VoiceTalker 리셋
	APlayerState* PS = GetPlayerState();
	if (PS == nullptr)
	{
		AC_LOG(LogSY, Warning, TEXT("PlayerState is nullptr"));
		return;
	}

	UVOIPStatics::ResetPlayerVoiceTalker(PS);

	// Deactivate만 하고 GC에 맡김 (오디오 스레드 안전)
	VOIPTalker->Deactivate();
	VOIPTalker = nullptr;

	AC_LOG(LogSY, Log, TEXT("  - VOIPTalker deactivated"));
}

void AACCharacter::MulticastPlayAttackMontage_Implementation()
{
	if (MeleeMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(MeleeMontage, 2.0f);
	}

	// 로컬 클라이언트 + 무기 장착 시에만 사운드 재생
	if (IsLocallyControlled() && BatSwingSound && ShopComponent)
	{
		UACItemData* EquippedWeapon = ShopComponent->EquippedWeapon;

		// 무기를 들고 있는지 확인
		if (EquippedWeapon != nullptr && EquippedWeapon->ItemType == EItemType::Weapon)
		{
			UGameplayStatics::PlaySound2D(this, BatSwingSound);
		}
	}
}

void AACCharacter::MulticastPlayEscapeSkillMontage_Implementation()
{
	if (EscapeMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(EscapeMontage, 4.0f);
	}

	// 로컬 클라이언트 + 무기 장착 시에만 사운드 재생
	if (IsLocallyControlled() && BatSwingSound && ShopComponent)
	{
		UACItemData* EquippedWeapon = ShopComponent->EquippedWeapon;

		// 무기를 들고 있는지 확인
		if (EquippedWeapon != nullptr && EquippedWeapon->ItemType == EItemType::Weapon)
		{
			UGameplayStatics::PlaySound2D(this, BatSwingSound);
		}
	}
}

void AACCharacter::ServerAttack_Implementation()
{
	PerformAttackTrace(); // 서버에서만 실행됨
}

void AACCharacter::ServerItemDrop_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Server ItemDrop!!"));
}

void AACCharacter::ServerFreezeCharacter_Implementation(AActor* Target)
{
	AC_LOG(LogSW, Error, TEXT("2222222"));
	//bOnInteract = true;
	SetCharacterState(ECharacterState::Interact);
	GetCharacterMovement()->StopMovementImmediately();

	if (Target == nullptr)
	{
		return;
	}
	// 2. 거리 검사 (가장 강력한 보안 장치)
	// 상호작용 가능한 최대 거리가 300이라면, 오차 범위 포함해서 350~400 정도로 검사
	// todo: 거리 다시 확인하기
	float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
	float MaxDistSq = FMath::Square(400.0f); // 400cm (4미터)

	if (DistSq > MaxDistSq)
	{
		AC_LOG(LogSW, Error, TEXT("해킹 의심: 너무 먼 거리의 타겟 요청. 거리: %f"), FMath::Sqrt(DistSq));
		return;
	}

	IACInteractInterface* Interactable = Cast<IACInteractInterface>(Target);
	if (Interactable == nullptr)
	{
		return;
	}

	if (Interactable->GetInteractorType() == EACInteractorType::Citizen || Interactable->GetInteractorType() == EACInteractorType::BlackMarketDealer)
	{
		ACharacter* Char = Cast<ACharacter>(Target);
		if (Char == nullptr)
		{
			return;
		}

		// SetMovementMode는 Replicated되므로 클라이언트에도 동기화됨
		Char->GetCharacterMovement()->SetMovementMode(MOVE_None);
		AC_LOG(LogSW, Error, TEXT("333333333"));
	}
	else
	{
		AACCharacter* ACChar = Cast<AACCharacter>(Target);
		if (ACChar == nullptr)
		{
			return;
		}

		ACChar->SetCharacterState(ECharacterState::OnInteract);
		ACChar->GetCharacterMovement()->StopMovementImmediately();
		AC_LOG(LogSW, Error, TEXT("44444444"));
	}
}

void AACCharacter::ServerUnfreezeCharacter_Implementation(AActor* Target)
{
	SetCharacterState(ECharacterState::Free);
	AC_LOG(LogSW, Error, TEXT("5555555555"));

	if (Target == nullptr)
	{
		return;
	}

	// 2. 거리 검사 (가장 강력한 보안 장치)
	// 상호작용 가능한 최대 거리가 300이라면, 오차 범위 포함해서 350~400 정도로 검사
	// todo: 거리 다시 확인하기
	float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
	float MaxDistSq = FMath::Square(400.0f); // 400cm (4미터)

	if (DistSq > MaxDistSq)
	{
		AC_LOG(LogSW, Error, TEXT("해킹 의심: 너무 먼 거리의 타겟 요청. 거리: %f"), FMath::Sqrt(DistSq));
		return;
	}

	IACInteractInterface* Interactable = Cast<IACInteractInterface>(Target);
	if (Interactable == nullptr)
	{
		return;
	}

	if (Interactable->GetInteractorType() == EACInteractorType::Citizen || Interactable->GetInteractorType() == EACInteractorType::BlackMarketDealer)
	{
		ACharacter* Char = Cast<ACharacter>(Target);
		if (Char == nullptr)
		{
			return;
		}

		// SetMovementMode는 Replicated되므로 클라이언트에도 동기화됨
		Char->GetCharacterMovement()->SetMovementMode(MOVE_NavWalking);
		AC_LOG(LogSW, Error, TEXT("666666666"));
	}
	else
	{
		AACCharacter* ACChar = Cast<AACCharacter>(Target);
		if (ACChar == nullptr)
		{
			return;
		}

		ACChar->SetCharacterState(ECharacterState::Free);
		AC_LOG(LogSW, Error, TEXT("777777777"));
	}
}

// === 홀드 상호작용 RPC 구현 ===
void AACCharacter::ServerStartHoldInteraction_Implementation(AActor* TargetActor, UACInteractionData* InteractionData)
{
	if (TargetActor == nullptr)
	{
		AC_LOG(LogSW, Error, TEXT("11111111"));
		return;
	}
	if (IsValid(TargetActor) == false)
	{
		AC_LOG(LogSW, Error, TEXT("22222222"));
		return;
	}
	if (InteractionData == nullptr)
	{
		AC_LOG(LogSW, Error, TEXT("33333333"));
		return;
	}

	// Multicast로 몽타주 재생 및 회전 동기화
	MulticastStartHoldInteraction(
		TargetActor,
		InteractionData->InitiatorMontage,
		InteractionData->TargetMontage,
		InteractionData->bIsCharacterInteraction
	);
}

void AACCharacter::ServerStopHoldInteraction_Implementation(AActor* TargetActor)
{
	MulticastStopHoldInteraction(TargetActor);
}

void AACCharacter::MulticastStartHoldInteraction_Implementation(
	AActor* TargetActor,
	UAnimMontage* InitiatorMontage,
	UAnimMontage* TargetMontage,
	bool bDoRotateTarget)
{
	// 0. 회전 처리 (클라이언트 시각적 동기화)
	if (TargetActor)
	{
		// 1) 나(주체)를 타겟 방향으로 회전
		FVector DirectionToTarget = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		if (!DirectionToTarget.IsZero())
		{
			SetActorRotation(DirectionToTarget.Rotation());
		}

		// 2) 캐릭터 간 상호작용이면 타겟도 나를 보게 회전
		if (bDoRotateTarget)
		{
			FVector DirectionToMe = -DirectionToTarget;
			TargetActor->SetActorRotation(DirectionToMe.Rotation());
		}
	}

	// 1. Initiator 몽타주 재생
	if (InitiatorMontage)
	{
		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_Play(InitiatorMontage);
			CurrentInteractionMontage = InitiatorMontage;
		}
	}

	// 2. Target 몽타주 재생 (ACharacter* 공통 - 시민도 플레이어도)
	ACharacter* TargetChar = Cast<ACharacter>(TargetActor);
	if (TargetMontage && TargetChar)
	{
		if (UAnimInstance* TargetAnimInst = TargetChar->GetMesh()->GetAnimInstance())
		{
			TargetAnimInst->Montage_Play(TargetMontage);
			TargetInteractionMontage = TargetMontage;
		}
	}
}

void AACCharacter::MulticastStopHoldInteraction_Implementation(AActor* TargetActor)
{
	// 1. Initiator 몽타주 정지
	if (CurrentInteractionMontage)
	{
		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_Stop(0.25f, CurrentInteractionMontage);
		}
		CurrentInteractionMontage = nullptr;
	}

	// 2. Target 몽타주 정지
	ACharacter* TargetChar = Cast<ACharacter>(TargetActor);
	if (TargetInteractionMontage && TargetChar)
	{
		if (UAnimInstance* TargetAnimInst = TargetChar->GetMesh()->GetAnimInstance())
		{
			TargetAnimInst->Montage_Stop(0.25f, TargetInteractionMontage);
		}
		TargetInteractionMontage = nullptr;
	}
}

EACCharacterType AACCharacter::GetCharacterType() const
{
	// todo: 우리 게임에 이 캐릭터 그대로 사용할 일이 있나?
	return EACCharacterType::None;
}

#pragma region Update SkeletalMeshComponent

// void AACCharacter::OnRep_HeadMesh() const
// {
// 	if (HeadMesh)
// 	{
// 		AC_LOG(LogHY, Error, TEXT("Before HeadMeshComp OK | prev:%s next:%s"), HeadMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *HeadMesh->GetSkeletalMeshAsset()->GetName(), *HeadMeshReal.GetName());
// 		UpdateHeadMesh();
// 		AC_LOG(LogHY, Error, TEXT("After HeadMeshComp OK | prev:%s next:%s"), HeadMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *HeadMesh->GetSkeletalMeshAsset()->GetName(), *HeadMeshReal.GetName());
// 	}
// }
//
// void AACCharacter::OnRep_FaceMesh() const
// {
// 	if (FaceMesh)
// 	{
// 		AC_LOG(LogHY, Error, TEXT("Before FaceMeshComp OK | prev:%s next:%s"), FaceMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *FaceMesh->GetSkeletalMeshAsset()->GetName(), *FaceMeshReal.GetName());
// 		UpdateFaceMesh();
// 		AC_LOG(LogHY, Error, TEXT("After FaceMeshComp OK | prev:%s next:%s"), FaceMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *FaceMesh->GetSkeletalMeshAsset()->GetName(), *FaceMeshReal.GetName());
// 	}
// }
//
// void AACCharacter::OnRep_TopMesh() const
// {
// 	if (TopMesh)
// 	{
// 		AC_LOG(LogHY, Error, TEXT("Before TopMeshComp OK | prev:%s next:%s"), TopMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *TopMesh->GetSkeletalMeshAsset()->GetName(), *TopMeshReal.GetName());
// 		UpdateTopMesh();
// 		AC_LOG(LogHY, Error, TEXT("After TopMeshComp OK | prev:%s next:%s"), TopMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *TopMesh->GetSkeletalMeshAsset()->GetName(), *TopMeshReal.GetName());
// 	}
// }
//
// void AACCharacter::OnRep_BottomMesh() const
// {
// 	if (BottomMesh)
// 	{
// 		AC_LOG(LogHY, Error, TEXT("Before BottomMeshComp OK | prev:%s next:%s"), BottomMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *BottomMesh->GetSkeletalMeshAsset()->GetName(), *BottomMeshReal.GetName());
// 		UpdateBottomMesh();
// 		AC_LOG(LogHY, Error, TEXT("After BottomMeshComp OK | prev:%s next:%s"), BottomMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *BottomMesh->GetSkeletalMeshAsset()->GetName(), *BottomMeshReal.GetName());
// 	}
// }
//
// void AACCharacter::OnRep_ShoesMesh() const
// {
// 	if (ShoesMesh)
// 	{
// 		AC_LOG(LogHY, Error, TEXT("Before ShoesMeshComp OK | prev:%s next:%s"), ShoesMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *ShoesMesh->GetSkeletalMeshAsset()->GetName(), *ShoesMeshReal.GetName());
// 		UpdateShoesMesh();
// 		AC_LOG(LogHY, Error, TEXT("After ShoesMeshComp OK | prev:%s next:%s"), ShoesMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *ShoesMesh->GetSkeletalMeshAsset()->GetName(), *ShoesMeshReal.GetName());
// 	}
// }
//
// void AACCharacter::OnRep_FaceAccMesh() const
// {
// 	if (FaceAccMesh)
// 	{
// 		AC_LOG(LogHY, Error, TEXT("Before ShoesMeshComp OK | prev:%s next:%s"), FaceAccMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *FaceAccMesh->GetSkeletalMeshAsset()->GetName(), *FaceAccMeshReal.GetName());
// 		UpdateFaceAccMesh();
// 		AC_LOG(LogHY, Error, TEXT("After ShoesMeshComp OK | prev:%s next:%s"), FaceAccMesh->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *FaceAccMesh->GetSkeletalMeshAsset()->GetName(), *FaceAccMeshReal.GetName());
// 	}
// }

void AACCharacter::ApplyHeadMesh()
{
	if (USkeletalMesh* SKMesh = HeadMeshReal.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before TopMeshComp OK | prev:%s next:%s"), TopMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *TopMeshComp->GetSkeletalMeshAsset()->GetName(), *TopMesh->GetName());
		HeadMesh->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After TopMeshComp OK | prev:%s next:%s"), *TopMeshComp->GetSkeletalMeshAsset()->GetName(), *TopMesh->GetName());
	}
}

void AACCharacter::ApplyFaceMesh()
{
	if (USkeletalMesh* SKMesh = FaceMeshReal.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before TopMeshComp OK | prev:%s next:%s"), TopMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *TopMeshComp->GetSkeletalMeshAsset()->GetName(), *TopMesh->GetName());
		FaceMesh->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After TopMeshComp OK | prev:%s next:%s"), *TopMeshComp->GetSkeletalMeshAsset()->GetName(), *TopMesh->GetName());
	}
}

void AACCharacter::ApplyTopMesh()
{
	if (USkeletalMesh* SKMesh = TopMeshReal.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before TopMeshComp OK | prev:%s next:%s"), TopMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *TopMeshComp->GetSkeletalMeshAsset()->GetName(), *TopMesh->GetName());
		TopMesh->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After TopMeshComp OK | prev:%s next:%s"), *TopMeshComp->GetSkeletalMeshAsset()->GetName(), *TopMesh->GetName());
	}
}

void AACCharacter::ApplyBottomMesh()
{
	if (USkeletalMesh* SKMesh = BottomMeshReal.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before BottomMeshComp OK | prev:%s next:%s"), BottomMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *BottomMeshComp->GetSkeletalMeshAsset()->GetName(), *BottomMesh->GetName());
		BottomMesh->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After BottomMeshComp OK | prev:%s next:%s"), *BottomMeshComp->GetSkeletalMeshAsset()->GetName(), *BottomMesh->GetName());
	}
}

void AACCharacter::ApplyShoesMesh()
{
	if (USkeletalMesh* SKMesh = ShoesMeshReal.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before ShoesMeshComp OK | prev:%s next:%s"), ShoesMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *ShoesMeshComp->GetSkeletalMeshAsset()->GetName(), *ShoesMesh->GetName());
		ShoesMesh->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After ShoesMeshComp OK | prev:%s next:%s"), *ShoesMeshComp->GetSkeletalMeshAsset()->GetName(), *ShoesMesh->GetName());
	}
}

void AACCharacter::ApplyFaceAccMesh()
{
	if (USkeletalMesh* SKMesh = FaceAccMeshReal.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before FaceAccMeshComp OK | prev:%s next:%s"), FaceAccMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *FaceAccMeshComp->GetSkeletalMeshAsset()->GetName(), *FaceAccMesh->GetName());
		FaceAccMesh->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After FaceAccMeshComp OK | prev:%s next:%s"), *FaceAccMeshComp->GetSkeletalMeshAsset()->GetName(), *FaceAccMesh->GetName());
	}
}

void AACCharacter::OnRep_HeadMesh()
{
	if (HeadMesh == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_HeadMesh: HeadMeshComp is NULL"));
		return; 
	}
	
	if (HeadMeshReal.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_HeadMesh: HeadMesh is NULL"));
		return;
	}

	if (HeadMeshReal.IsValid())
	{
		ApplyHeadMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다1"));	
	}
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(HeadMeshReal.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCharacter::ApplyHeadMesh));
}

void AACCharacter::OnRep_FaceMesh()
{
	if (FaceMesh == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_FaceMesh: FaceMeshComp is NULL"));
		return;
	}

	if (FaceMeshReal.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_FaceMesh: FaceMesh is NULL"));
		return;
	}

	if (FaceMeshReal.IsValid())
	{
		ApplyFaceMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다2"));	
	}
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(FaceMeshReal.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCharacter::ApplyFaceMesh));
}

void AACCharacter::OnRep_TopMesh()
{
	if (TopMesh == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_TopMesh: TopMeshComp is NULL"));
		return;
	}
	
	if (TopMeshReal.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_TopMesh: TopMesh is NULL"));
		return;
	}

	if (TopMeshReal.IsValid())
	{
		ApplyTopMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다3"));	
	}
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(TopMeshReal.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCharacter::ApplyTopMesh));
}

void AACCharacter::OnRep_BottomMesh()
{
	if (BottomMesh == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_BottomMesh: BottomMeshComp is NULL"));
		return; 
	}
	
	if (BottomMeshReal.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_BottomMesh: BottomMesh is NULL"));
		return;
	}

	if (BottomMeshReal.IsValid())
	{
		ApplyBottomMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다4"));	
	}
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(BottomMeshReal.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCharacter::ApplyBottomMesh));
}

void AACCharacter::OnRep_ShoesMesh()
{
	if (ShoesMesh == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_ShoesMesh: ShoesMeshComp is NULL"));
		return;
		
	}
	if (ShoesMeshReal.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_ShoesMesh: ShoesMesh is NULL"));
		return;
	}

	if (ShoesMeshReal.IsValid())
	{
		ApplyShoesMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다5"));	
	}
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(ShoesMeshReal.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCharacter::ApplyShoesMesh));
}

void AACCharacter::OnRep_FaceAccMesh()
{
	if (FaceAccMesh == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_FaceAccMesh: FaceAccMeshComp is NULL"));
		return;
	}
	
	if (FaceAccMeshReal.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_FaceAccMesh: FaceAccMesh is NULL"));
		return;
	}

	if (FaceAccMeshReal.IsValid())
	{
		ApplyFaceAccMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다6"));	
	}
	
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(FaceAccMeshReal.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCharacter::ApplyFaceAccMesh));
}

#pragma endregion


// #####################################################################################
// ########################## 나중에 사라져야할 코드 ######################################
// #####################################################################################

void AACCharacter::ServerShoot_Implementation()
{
	SpendBullets(1);

	UCameraComponent* ActiveCamera = GunCamera;


	FVector CameraLoc;
	FRotator CameraRot;
	float MaxDistance = 2000;
	GetOwner()->GetActorEyesViewPoint(CameraLoc, CameraRot);
	AC_LOG(LogHY, Log, TEXT("Firing Hitscan!! Who:%s %s"), *CameraLoc.ToString(), *CameraRot.ToString());
	CameraLoc = ActiveCamera->GetComponentLocation();
	CameraRot = ActiveCamera->GetComponentRotation();

	AC_LOG(LogHY, Log, TEXT("Firing Hitscan!! Gun:%s %s"), *CameraLoc.ToString(), *CameraRot.ToString());
	AC_LOG(LogHY, Log, TEXT("Firing Hitscan!! Follow:%s %s"), *FollowCamera->GetComponentLocation().ToString(), *FollowCamera->GetComponentRotation().ToString());

	FVector TraceEnd = CameraLoc + (CameraRot.Vector() * MaxDistance);

	//AC_LOG(LogHY, Error, TEXT("Shoot Test"));
	// MulticastPlayShootMontage();
	// 총구 위치
	// 방안 [소켓으로부터 출발] [SkeletalMesh의 경우 거기서]
	//FVector MuzzleLoc = GetMesh()->GetSocketLocation("RightHandSocket");

	FVector MuzzleLoc = CameraLoc;
	FVector ShootDir = (TraceEnd - MuzzleLoc).GetSafeNormal();
	FVector End = MuzzleLoc + (ShootDir * MaxDistance);

	FHitResult Hit;

	FCollisionObjectQueryParams ObjectParams;

	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);
	ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel6);
	ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel7);
	ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel8);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bReturnPhysicalMaterial = true;

	bool bHit = GetWorld()->LineTraceSingleByObjectType(Hit, MuzzleLoc, End, ObjectParams, QueryParams);

	// FColor LineColor = bHit ? FColor::Red : FColor::Green;
	// DrawDebugLine(GetWorld(),MuzzleLoc, bHit ? Hit.ImpactPoint : End, LineColor, false, 2.0f, 0, 2.0f);
	if (bHit)
	{
		UE_LOG(LogTemp, Log, TEXT("Hit: %s"), *Hit.GetActor()->GetName());
		UGameplayStatics::ApplyDamage(Hit.GetActor(), GunDamage, GetController(), this, AACGunBase::StaticClass());
	}
}

void AACCharacter::OnRep_SprintGauge()
{
	OnSprintChanged.Broadcast(SprintGauge);
	if (SprintGauge == 10)
	{
		OnSprintUIHide.Broadcast();
	}
	else
	{
		OnSprintUIShow.Broadcast();
	}
}

bool AACCharacter::CanZoomIn()
{
	if (CharacterState == ECharacterState::None || CharacterState == ECharacterState::OnDamage ||
		CharacterState == ECharacterState::Interact || CharacterState == ECharacterState::OnInteract ||
		CharacterState == ECharacterState::Stun)
	{
		AC_LOG(LogHY, Log, TEXT("Cant ZoomIn %s"), *UEnum::GetValueAsString(CharacterState));
		return false;
	}

	return true;
}


void AACCharacter::ServerPlayZoomIn_Implementation()
{
	MulticastPlayZoomMontage();
}

void AACCharacter::ServerPlayZoomOut_Implementation()
{
	MulticastStopAllMontage();
}

void AACCharacter::PlayHitEffect(float Duration)
{
	// Overlay Material이 없으면 리턴
	if (DamageOverlayMaterial == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("DamageOverlayMaterial is null! Please assign in Blueprint."));
		return;
	}


	// 모든 메시 컴포넌트 리스트
	TArray<USkeletalMeshComponent*> MeshComponents = {
		HeadMesh,
		FaceMesh,
		TopMesh,
		BottomMesh,
		ShoesMesh,
		FaceAccMesh,
		GetMesh()
	};

	// 각 메시에 Overlay Material 적용
	for (USkeletalMeshComponent* MeshComp : MeshComponents)
	{
		if (MeshComp != nullptr)
		{
			MeshComp->SetOverlayMaterial(DamageOverlayMaterial);
		}
	}

	// 타이머로 Overlay 제거 예약
	GetWorld()->GetTimerManager().SetTimer(
		HitEffectTimerHandle,
		this,
		&AACCharacter::ResetHitEffect,
		Duration,
		false  // 한 번만 실행
	);
}

void AACCharacter::ResetHitEffect()
{
	if (IsValid(this) == false)
	{
		AC_LOG(LogHY, Error, TEXT("this is nullptr"));
		return;
	}

	// 스턴 상태면 Overlay Material 제거하지 않음
	 // 만약 타이머 클리어가 실패했거나 타이밍 이슈로
	 // ResetHitEffect()가 호출되더라도 스턴 오버레이는 유지
	if (bStun == true)
	{
		AC_LOG(LogHG, Warning, TEXT("Stun active, keeping StunOverlay"));
		return;
	}

	// 모든 메시 컴포넌트 리스트
	TArray<USkeletalMeshComponent*> MeshComponents = {
		HeadMesh,
		FaceMesh,
		TopMesh,
		BottomMesh,
		ShoesMesh,
		FaceAccMesh,
		GetMesh()
	};

	// 각 메시의 Overlay Material 제거
	for (USkeletalMeshComponent* MeshComp : MeshComponents)
	{
		if (MeshComp != nullptr)
		{
			MeshComp->SetOverlayMaterial(nullptr);
		}
	}
	AC_LOG(LogHY, Error, TEXT("Reset Test"));
}

void AACCharacter::OnRep_Stun()
{
	if (bStun == true)
	{
		ApplyStunOverlay();
	}
	else
	{
		RemoveStunOverlay();
	}
}

void AACCharacter::ApplyStunOverlay()
{
	if (StunOverlayMaterial == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("StunOverlayMaterial is null!"));
		return;
	}

	// 모든 메시 컴포넌트
	TArray<USkeletalMeshComponent*> MeshComponents = {
		HeadMesh,
		FaceMesh,
		TopMesh,
		BottomMesh,
		ShoesMesh,
		FaceAccMesh,
		GetMesh()
	};

	// 각 메시에 Overlay Material 적용
	for (USkeletalMeshComponent* MeshComp : MeshComponents)
	{
		if (MeshComp != nullptr)
		{
			MeshComp->SetOverlayMaterial(StunOverlayMaterial);
		}
	}

	AC_LOG(LogHG, Log, TEXT("Stun overlay applied"));
}

void AACCharacter::RemoveStunOverlay()
{
	// 모든 메시 컴포넌트
	TArray<USkeletalMeshComponent*> MeshComponents = {
		HeadMesh,
		FaceMesh,
		TopMesh,
		BottomMesh,
		ShoesMesh,
		FaceAccMesh,
		GetMesh()
	};

	// 각 메시의 Overlay Material 제거
	for (USkeletalMeshComponent* MeshComp : MeshComponents)
	{
		if (MeshComp != nullptr)
		{
			MeshComp->SetOverlayMaterial(nullptr);
		}
	}

	AC_LOG(LogHG, Log, TEXT("Stun overlay removed"));
}

// PlayerController가 실행하는 함수.
void AACCharacter::ServerPlayShootMontage_Implementation()
{
	if (IsHoldingGun() == false)
	{
		return;
	}

	if (GetBulletCount() <= 0)
	{
		AC_LOG(LogTemp, Log, TEXT("총알 갯수 %d"), GetBulletCount());
		return;
	}

	if (bIsCarry == true)
	{
		AC_LOG(LogSY, Log, TEXT("Cannot interact while carrying"));
		return;
	}
	
	// 실행 중인지 확인.
	if (GetMesh()->GetAnimInstance()->Montage_IsActive(ShootMontage) == true)
	{
		AC_LOG(LogHY, Log, TEXT("몽타주가 재생 중 입니다."));
		return;
	}
	
	MulticastPlayShootMontage();
}

int32 AACCharacter::GetBulletCount() const
{
	return BulletCount;
}

void AACCharacter::AddBullets(int32 InBulletCount)
{
	AC_LOG(LogHY, Error, TEXT("Current Bullet Count: %d Input BulletCount: %d"), BulletCount, InBulletCount);
	BulletCount += InBulletCount;
	OnRep_BulletCount();
	// AACMainPlayerController* MainPlayerController = Cast<AACMainPlayerController>(GetController());
	// if (MainPlayerController == nullptr)
	// {
	// 	return;
	// }
	// if (MainPlayerController->ACHUDWidget == nullptr)
	// {
	// 	AC_LOG(LogHY, Log, TEXT("ACHUDWidget is nullptr"));
	// 	return;
	// }
	// MainPlayerController->ACHUDWidget->HandleAmmoChanged(BulletCount);
}

void AACCharacter::ClearBullets()
{
	BulletCount = 0;
	OnRep_BulletCount();
	// AACMainPlayerController* MainPlayerController = Cast<AACMainPlayerController>(GetController());
	// if (MainPlayerController == nullptr)
	// {
	// 	return;
	// }
	// MainPlayerController->ACHUDWidget->HandleAmmoChanged(BulletCount);
}

void AACCharacter::SpendBullets(int32 InBulletCount)
{
	if (InBulletCount <= 0)
	{
		AC_LOG(LogHY, Error, TEXT("Input BulletCount is Zero"));
		return;
	}

	if (BulletCount < InBulletCount)
	{
		AC_LOG(LogHY, Error, TEXT("Current Bullet Count: %d Input BulletCount: %d"), BulletCount, InBulletCount);
		return;
	}
	BulletCount -= InBulletCount;
	OnRep_BulletCount();

	// AACMainPlayerController* MainPlayerController = Cast<AACMainPlayerController>(GetController());
	// if (MainPlayerController == nullptr)
	// {
	// 	return;
	// }
	// MainPlayerController->ACHUDWidget->HandleAmmoChanged(BulletCount);
}

void AACCharacter::OnRep_BulletCount()
{
	AACMainPlayerController* PC = GetMainPlayerController();
	if (PC && PC->IsLocalController())
	{
		PC->UpdateAmmoUI(BulletCount);
	}
}

void AACCharacter::Server_SetZoomInState_Implementation(bool bPlay)
{
	Multicast_SetZoomInState(bPlay);
}

void AACCharacter::Multicast_SetZoomInState_Implementation(bool bPlay)
{
	UACCharacterAnimInstance* Anim = Cast<UACCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (Anim == nullptr)
	{
		return;
	}
	bIsZoomIn = bPlay;
	Anim->SetIsZoomIn(bPlay);
}

void AACCharacter::SetZoomInState(bool bPlay)
{
	if (HasAuthority())
	{
		Multicast_SetZoomInState(bPlay);
	}
	else
	{
		Server_SetZoomInState(bPlay);
	}
}

void AACCharacter::OnRep_VoiceGroup()
{
	// 로컬 플레이어 캐릭터가 무전기 상태 변경 시 모든 다른 캐릭터의 VOIP 설정 업데이트
	if (IsLocallyControlled() == true)
	{
		UpdateRadioVoiceSettings();
	}
	else
	{
		// 다른 캐릭터의 무전기 상태가 변경됨 → 로컬 플레이어가 무전기 있으면 해당 캐릭터 VOIP 업데이트
		APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
		if (LocalPC == nullptr)
		{
			return;
		}

		AACCharacter* LocalChar = Cast<AACCharacter>(LocalPC->GetPawn());
		if (LocalChar == nullptr)
		{
			return;
		}

		if (VoiceGroup != EVoiceGroup::None && LocalChar->VoiceGroup == VoiceGroup)
		{
			// 나(로컬)도 무전기 있고, 상대도 무전기 있으면서 같은 무전기 그룹이면 라디오 모드
			AC_LOG(LogSY, Log, TEXT("Radio Mode: %s"), *GetName());
			SetVOIPMode(true);
		}
		else
		{
			// 거리 감쇠 모드 다시 적용
			AC_LOG(LogSY, Log, TEXT("Proximity Mode: %s"), *GetName());
			SetVOIPMode(false);
		}
	}
}

void AACCharacter::SetVoiceGroup(EVoiceGroup NewVoiceGroup)
{
	if (VoiceGroup == NewVoiceGroup)
	{
		return;
	}

	VoiceGroup = NewVoiceGroup;
	AC_LOG(LogVT, Log, TEXT("SetVoiceGroup - Character: %s NewVoiceGroup: %s"), *GetName(), *UEnum::GetValueAsString(NewVoiceGroup));
	// 클라이언트들 → 자동으로 OnRep 호출됨
	// 서버(Listen Server 호스트) → 수동 호출 필요
	if (HasAuthority() == true)
	{
		OnRep_VoiceGroup();
	}
}

void AACCharacter::UpdateRadioVoiceSettings()
{
	// 로컬 플레이어만 호출
	if (IsLocallyControlled() == false)
	{
		return;
	}

	// 모든 ACCharacter를 순회하며 VOIP 설정 업데이트
	for (TActorIterator<AACCharacter> It(GetWorld()); It; ++It)
	{
		AACCharacter* OtherChar = *It;
		if (OtherChar == this)
		{
			continue;
		}

		// 나도 무전기 있고 상대도 무전기 있으면서 같은 그룹이면 → 라디오 모드
		if (VoiceGroup != EVoiceGroup::None && VoiceGroup == OtherChar->VoiceGroup)
		{
			OtherChar->SetVOIPMode(true);
		}
		else
		{
			OtherChar->SetVOIPMode(false);
		}
	}
}

void AACCharacter::SetVOIPMode(bool bUseRadio)
{
	if (VOIPTalker == nullptr)
	{
		return;
	}


	USoundAttenuation* NewAttenuation = bUseRadio ? VoiceRadio : VoiceProximity;

	// 이미 같은 설정이면 스킵
	if (VOIPTalker->Settings.AttenuationSettings == NewAttenuation)
	{
		return;
	}

	VOIPTalker->Settings.AttenuationSettings = NewAttenuation;

	// 재등록하여 변경사항 즉시 적용
	APlayerState* PS = GetPlayerState();
	if (PS != nullptr)
	{
		VOIPTalker->RegisterWithPlayerState(PS);
	}
}
