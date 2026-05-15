// Fill out your copyright notice in the Description page of Project Settings.


#include "ACCitizen.h"

#include "ACCharacter.h"
#include "ACMafiaCharacter.h"
#include "ACPoliceCharacter.h"
#include "AnimalCrime.h"
#include "BrainComponent.h"
#include "NavigationSystem.h"
#include "AI/ACCitizenAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/ACInteractableComponent.h"
#include "Component/ACMoneyComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/Interaction/ACInteractionInfoWidget.h"
#include "Interaction/ACInteractionData.h"
#include "Game/ACPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "AnimalCrime.h"
#include "CharacterStat/ACCitizenData.h"
#include "Engine/AssetManager.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "Engine/StreamableManager.h"
#include "Game/ACGameRuleManager.h"
#include "Game/ACMainGameMode.h"
#include "Game/ACMainPlayerController.h"


// Sets default values
AACCitizen::AACCitizen()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// AI 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AACCitizenAIController::StaticClass();

	bReplicates = true;
	SetReplicateMovement(true);
	
	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("CitizenCollision"));

	//스켈레탈 메시
	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Body_010.SK_Body_010")));
	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	MeshComp->SetIsReplicated(true); // 반드시 추가

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimRef(TEXT("/Script/Engine.AnimBlueprint'/Game/Project/Character/ABP_ACPlayer.ABP_ACPlayer_C'"));
	if (AnimRef.Succeeded())
	{
		MeshComp->SetAnimInstanceClass(AnimRef.Class);
	}
	
	static ConstructorHelpers::FObjectFinder<UAnimMontage> DamagedMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/Project/Character/AM_OnDamaged.AM_OnDamaged'"));
	if (DamagedMontageRef.Succeeded())
	{
		DamagedMontage = DamagedMontageRef.Object;
	}
	
	
	GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
	GetCharacterMovement()->MovementMode = EMovementMode::MOVE_NavWalking;
	
	HeadMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	HeadMeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	HeadMeshComp->SetupAttachment(RootComponent);
	HeadMeshComp->SetLeaderPoseComponent(MeshComp);
	HeadMeshComp->SetReceivesDecals(false);
	HeadMeshComp->SetIsReplicated(true);
	
	FaceMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceMesh"));
	FaceMeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	FaceMeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	FaceMeshComp->SetupAttachment(RootComponent);
	FaceMeshComp->SetLeaderPoseComponent(MeshComp);
	FaceMeshComp->SetReceivesDecals(false);
	FaceMeshComp->SetIsReplicated(true);
	
	
	TopMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TopMesh"));
	TopMeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	TopMeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	TopMeshComp->SetupAttachment(RootComponent);
	TopMeshComp->SetLeaderPoseComponent(MeshComp);
	TopMeshComp->SetReceivesDecals(false);
	TopMeshComp->SetIsReplicated(true);
	

	BottomMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BottomMesh"));
	BottomMeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	BottomMeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	BottomMeshComp->SetupAttachment(RootComponent);
	BottomMeshComp->SetLeaderPoseComponent(MeshComp);
	BottomMeshComp->SetReceivesDecals(false);
	BottomMeshComp->SetIsReplicated(true);
	
	ShoesMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shoes"));
	ShoesMeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	ShoesMeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	ShoesMeshComp->SetupAttachment(RootComponent);
	ShoesMeshComp->SetLeaderPoseComponent(MeshComp);
	ShoesMeshComp->SetReceivesDecals(false);
	ShoesMeshComp->SetIsReplicated(true);

	FaceAccMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceAcc"));
	FaceAccMeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	FaceAccMeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	FaceAccMeshComp->SetupAttachment(RootComponent);
	FaceAccMeshComp->SetLeaderPoseComponent(MeshComp);
	FaceAccMeshComp->SetReceivesDecals(false);
	FaceAccMeshComp->SetIsReplicated(true);
	
	
	// 최적화 하기.
	HeadMeshComp->PrimaryComponentTick.bCanEverTick = false;
	FaceMeshComp->PrimaryComponentTick.bCanEverTick = false;
	TopMeshComp->PrimaryComponentTick.bCanEverTick = false;
	BottomMeshComp->PrimaryComponentTick.bCanEverTick = false;
	ShoesMeshComp->PrimaryComponentTick.bCanEverTick = false;
	FaceAccMeshComp->PrimaryComponentTick.bCanEverTick = false;
	
	// 애니메이션 몽타주
	static ConstructorHelpers::FObjectFinder<UAnimMontage> MeleeMontageRef(TEXT("/Game/Project/Character/AM_AIMelee.AM_AIMelee"));
	if (MeleeMontageRef.Succeeded())
	{
		MeleeMontage = MeleeMontageRef.Object;
	}
	
	// 인터랙션 컴포넌트
	InteractBoxComponent = CreateDefaultSubobject<UACInteractableComponent>(TEXT("InteractBoxComponent"));
	InteractBoxComponent->SetupAttachment(RootComponent);

	// 상호작용 위젯 컴포넌트
	InteractionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	InteractionWidgetComponent->SetupAttachment(RootComponent);
	InteractionWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionWidgetComponent->SetDrawSize(FVector2D(300.0f, 100.0f));
	InteractionWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // 머리 위
	InteractionWidgetComponent->SetVisibility(false); // 기본 숨김

	// 상호작용 위젯 클래스 설정
	static ConstructorHelpers::FClassFinder<UACInteractionInfoWidget> InteractionWidgetRef(
		TEXT("/Game/Project/UI/Interaction/WBP_InteractionInfo.WBP_InteractionInfo_C"));
	if (InteractionWidgetRef.Succeeded())
	{
		InteractionInfoWidgetClass = InteractionWidgetRef.Class;
	}
	
	MoneyComp = CreateDefaultSubobject<UACMoneyComponent>(TEXT("MoneyComponent"));
	
	//GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths = true;
	
	
	// 데이터 에셋
	static ConstructorHelpers::FObjectFinder<UACCitizenData> CitizenDataAssetRef(TEXT("/Script/AnimalCrime.ACCitizenData'/Game/Project/Character/DA_CitizenData.DA_CitizenData'"));
	if (CitizenDataAssetRef.Succeeded())
	{
		CitizenDataAsset = CitizenDataAssetRef.Object;
	}
}

#pragma region 엔진 제공 함수

void AACCitizen::PostInitializeComponents()
{
	//AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::PostInitializeComponents();
	
	DebugDeltaData = CitizenDataAsset->DebugDelta;
	
	DebugDrawLifeTimeData = CitizenDataAsset->DebugDrawLifeTime;
	RegenRateMinData = CitizenDataAsset->RegenRateMin;
	RegenRateMaxData = CitizenDataAsset->RegenRateMax;
	DropMoneyMinData = CitizenDataAsset->DropMoneyMin;
	DropMoneyMaxData = CitizenDataAsset->DropMoneyMax;
	DropTimeData = CitizenDataAsset->DropTime;
	AttackCapsuleRadiusData = CitizenDataAsset->AttackCapsuleRadius;
	AttackCapsuleHalfHeightData = CitizenDataAsset->AttackCapsuleHalfHeight;
	AttackTraceDistanceData = CitizenDataAsset->AttackTraceDistance;
	AttackDamageData = CitizenDataAsset->AttackDamage;
	DetectRadiusData = CitizenDataAsset->DetectRadius;
	
	
	DefaultWalkSpeedData = CitizenDataAsset->DefaultWalkSpeed;
	RunWalkSpeedData = CitizenDataAsset->RunWalkSpeed;
	AngryWalkSpeedData = CitizenDataAsset->AngryWalkSpeed;
	
	MafiaScoreData = CitizenDataAsset->MafiaScore;
	PoliceScoreData = CitizenDataAsset->PoliceScore;
	
	DebugDelta = DebugDeltaData; 
	//AC_LOG(LogHY, Warning, TEXT("End"));
}

void AACCitizen::PostNetInit()
{
	//AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::PostNetInit();
	//AC_LOG(LogHY, Warning, TEXT("End"));
}

void AACCitizen::BeginPlay()
{
	//AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::BeginPlay();
	
	// 시민 이동 속도 변경
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeedData;
	
	MoneyComp->InitMoneyComponent(EMoneyType::MoneyCitizenType);
	
	//float RandomRate = FMath::RandRange(0.f, 10.f);
	float RandomRate = 40.0f;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AACCitizen::UpdateAISkillFlag);
	GetWorld()->GetTimerManager().SetTimer(InitialSkillBlockTimerHandle, TimerDelegate, RandomRate, false);

	// 최적화
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		// 1. URO 활성화 스위치
		MeshComp->bEnableUpdateRateOptimizations = true;
	
		// 2. 가시성 기반 최적화 (매우 중요)
		// 화면에 안 보이면 애니메이션 업데이트를 하지 않거나 몽타주만 재생하도록 설정
		MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	}
	
	//AC_LOG(LogHY, Warning, TEXT("End"));
}

void AACCitizen::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//AC_LOG(LogHY, Warning, TEXT("AACCitizen::EndPlay"));
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

void AACCitizen::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	DebugDelta -= DeltaTime;
	if (DebugDelta <= 0.0f)
	{
		//AC_LOG(LogHY, Warning, TEXT("Damage Count: %d"), DamagedFlag);
		DebugDelta = DebugDeltaData;
	}
}

void AACCitizen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AACCitizen, HeadMesh);
	DOREPLIFETIME(AACCitizen, FaceMesh);
	DOREPLIFETIME(AACCitizen, TopMesh);
	DOREPLIFETIME(AACCitizen, BottomMesh);
	DOREPLIFETIME(AACCitizen, ShoesMesh);
	DOREPLIFETIME(AACCitizen, FaceAccMesh);
	
	DOREPLIFETIME(AACCitizen, CharacterState);
}

float AACCitizen::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float SuperDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	// 피격 사운드 재생 
	if (HitSound && DamageAmount > 0.0f)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	// Multicast로 모든 클라이언트에 피격 이펙트 전파
	if (DamageAmount > 0.0f)
	{
		// todo: 시민 OnDamage 효과 시간
		MulticastPlayHitEffect(2.0f);
	}

	AACCitizenAIController* AIControler = Cast<AACCitizenAIController>(GetController());
	
	// AIControler 여부 확인
	if (AIControler == nullptr)
	{
		return 0.0f;
	}
	
	// BlackBoard 여부 확인
	UBlackboardComponent* BBComp = AIControler->GetBlackboardComponent();
	if (BBComp == nullptr)
	{
		return 0.0f;
	}
	
	FVector RunPosition = GetRunPosition(DamageCauser->GetActorLocation());
	BBComp->SetValueAsVector("RunPosition", RunPosition);
	LastHitTime = GetWorld()->GetTimeSeconds();
	BBComp->SetValueAsFloat("LastHitTime", LastHitTime);
	OnDamaged();
	OnUpdateScore(DamageCauser);
	OnUpdateMoney(DamageCauser);
	OnChangeState();

	return SuperDamage;
}

#pragma endregion

void AACCitizen::PlayDamagedMontage(const FVector& Attack)
{
	FVector ForwardVector = GetActorForwardVector();
	FVector RightVector = GetActorRightVector();
	FVector DirVector = (Attack - GetActorLocation()).GetSafeNormal2D();
	float FrontBackResult = ForwardVector.Dot(DirVector);
	float RightLeftResult = RightVector.Dot(DirVector);
	
	GetMesh()->GetAnimInstance()->Montage_Play(DamagedMontage);
	if (FrontBackResult >= 0)
	{
		if (RightLeftResult >= 0)
		{
			// Front
			if (FrontBackResult >= RightLeftResult)
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Front"), DamagedMontage);
				
				UE_LOG(LogTemp, Log, TEXT("Front - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
			else
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Right"), DamagedMontage);

				UE_LOG(LogTemp, Log, TEXT("Right - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
		}
		else
		{
			// Front
			if (FrontBackResult >= abs(RightLeftResult))
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Front"), DamagedMontage);

				UE_LOG(LogTemp, Log, TEXT("Front - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
			else
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Left"), DamagedMontage);

				UE_LOG(LogTemp, Log, TEXT("Left - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
		}
	}
	else
	{
		if (RightLeftResult >= 0)
		{
			// Front
			if (abs(FrontBackResult) >= abs(RightLeftResult))
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Back"), DamagedMontage);

				UE_LOG(LogTemp, Log, TEXT("Back - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
			else
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Right"), DamagedMontage);

				UE_LOG(LogTemp, Log, TEXT("Right - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
		}
		else
		{
			// Front
			if (abs(FrontBackResult) >= abs(RightLeftResult))
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Back"), DamagedMontage);

				UE_LOG(LogTemp, Log, TEXT("Back - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
			else
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Left"), DamagedMontage);
				UE_LOG(LogTemp, Log, TEXT("Left - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
		}
		
	}
}

FVector AACCitizen::GetNextPosition() const
{
	// 현재 액터의 위치
	FVector CurrentLocation = GetActorLocation();

	// Navigation System 가져오기
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys == nullptr)
	{
		return CurrentLocation;
	}

	// 반환할 NavMesh 랜덤 위치
	FNavLocation RandomLocation;
    
	// 반경 (원하는 만큼 조절)
	float Radius = 2000.0f;

	// NavMesh에서 랜덤 reachable 포인트 얻기
	NavSys->GetRandomReachablePointInRadius(CurrentLocation, Radius, RandomLocation);

	return RandomLocation.Location;
}

FVector AACCitizen::GetRunPosition(const FVector& Attack) const
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("No navigation found1"));
		return FVector(0.0f, 0.0f, 0.0f);
	}
	
	FNavLocation Result;
	FVector MyLocation = GetActorLocation();
	FVector DirVector = (MyLocation - Attack).GetSafeNormal2D();
	FVector Desired = MyLocation + (DirVector * 3000.f);
	bool bFound = NavSys->ProjectPointToNavigation(Desired, Result, FVector(1000.f, 1000.f, 200.f));
	if (bFound == false)
	{
		AC_LOG(LogHY, Warning, TEXT("No navigation found2"));
		return FVector(0.0f, 0.0f, 0.0f);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("(%f, %f, %f)"), Result.Location.X, Result.Location.Y, Result.Location.Z);
	return Result.Location;
}

void AACCitizen::OnDamaged()
{
	DamagedFlag += 1;
	if (DamagedFlag > 0)
	{
		AAIController* AICon = GetController<AAIController>();
		AICon->GetBrainComponent()->StopLogic("HitAbort");
		AICon->GetBrainComponent()->StartLogic();
	}
	
	
	// 임시값을 저장.
	TempCharacterState = ECharacterState::OnDamage;
}

void AACCitizen::OnArrive()
{
	DamagedFlag = 0;
}

void AACCitizen::OnChangeState()
{
	// 상태값 변경
	//AC_LOG(LogHY, Warning, TEXT("Cur: %d Next: %d"), CharacterState, TempCharacterState );
	CharacterState = TempCharacterState;
	OnRep_CharacterState();
	
	if (CharacterState == ECharacterState::OnDamage)
	{
		// todo: 시민 OnDamage 타이머
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &AACCitizen::UpdateCharacterStatusFree);
		GetWorld()->GetTimerManager().SetTimer(MoveSpeedTimerHandle, TimerDelegate, 2.0, false);
	}
	else if (CharacterState == ECharacterState::Angry)
	{
		MoveSpeedTimerHandle.Invalidate();
		AACCitizenAIController* AICon = Cast<AACCitizenAIController>(GetController());
		if (AICon == nullptr)
		{
			return;
		}

		UBlackboardComponent* BBComp = AICon->GetBlackboardComponent();
		if (BBComp == nullptr)
		{
			return;
		}

		//AC_LOG(LogHY, Warning, TEXT("Next Target: %s"), *MafiaCharacter->GetName());
		BBComp->SetValueAsObject(TEXT("Target"), MafiaCharacter);
	}
}

void AACCitizen::OnUpdateScore(AActor* Actor)
{
	// Actor 체크
	if (Actor == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("Actor is nullptr"));
		return;
	}
	
	AACMainGameMode* GameMode = Cast<AACMainGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	// GameMode 체크
	if (GameMode == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("GameMode is nullptr"));
		return;
	}

	//  
	AACCharacter* ACCharacter = Cast<AACCharacter>(Actor);
	if (ACCharacter == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("ACCharacter is nullptr"));
		return;
	}
	
	EACCharacterType CharacterType = ACCharacter->GetCharacterType();
	switch (CharacterType)
	{
		case EACCharacterType::Police:
		{
			GameMode->UpdateGameScoreFromPolice(EPoliceAction::AttackCivilian,PoliceScoreData);
			break;
		}
		case EACCharacterType::Mafia:
		{
			GameMode->UpdateGameScoreFromMafia(EMafiaAction::AttackCivilian,MafiaScoreData);
			break;
		}
	}
}

void AACCitizen::OnUpdateMoney(AActor* Actor)
{
	// Actor 체크
	if (Actor == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("Actor is nullptr"));
		return;
	}
	
	AACMainGameMode* GameMode = Cast<AACMainGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	// GameMode 체크
	if (GameMode == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("GameMode is nullptr"));
		return;
	}

	//  
	AACCharacter* ACCharacter = Cast<AACCharacter>(Actor);
	if (ACCharacter == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("ACCharacter is nullptr"));
		return;
	}
	
	//int32 Money = 500;// FMath::RandRange(1, 100);
	int32 Money = FMath::RandRange(DropMoneyMinData, DropMoneyMaxData);
	EACCharacterType CharacterType = ACCharacter->GetCharacterType();
	switch (CharacterType)
	{
		case EACCharacterType::Police:
		{
			MoneyComp->EarnMoney(Money);
			ACCharacter->MoneyComp->LoseMoney(Money);
			//AC_LOG(LogHY, Warning, TEXT("Police Lost Money: %d Cur Money: %d"), Money, ACCharacter->MoneyComp->GetMoney());
			return;
		}
		case EACCharacterType::Mafia:
		{
			if (GetWorld()->GetTimerManager().IsTimerActive(MoneyCoolTimerHandle) == true)
			{
				return ;
			}
			
			int32 Result = MoneyComp->LoseMoney(Money);
			int32 CurrentMoney = MoneyComp->GetMoney();
			if (CurrentMoney <= 0)
			{
				TempCharacterState = ECharacterState::Angry;
				MafiaCharacter = Cast<AACMafiaCharacter>(Actor);
				
				TryRegenMoneyTimer();
				//AC_LOG(LogHY, Error, TEXT("CurrentMoney <= 0  name:%s"), *MafiaCharacter->GetName());
			}
			else
			{
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindUObject(this, &AACCitizen::ExcuteMoneyCoolTime);
				
				// AC_Log 출력 시 발생하는 타이머 버그 수정
				GetWorldTimerManager().SetTimer(MoneyCoolTimerHandle, TimerDelegate, DropTimeData, false);
				ACCharacter->MoneyComp->EarnMoney(Result);
			}
			//AC_LOG(LogHY, Warning, TEXT("Mafia Earn Money: %d Cur Money: %d"), Result, ACCharacter->MoneyComp->GetMoney());
			
			break;
		}
	}
}

float AACCitizen::GetLastHitTime() const
{
	return LastHitTime;
}

void AACCitizen::AttackHitCheck()
{
	// 캡슐 크기
	float CapsuleRadius = AttackCapsuleRadiusData;
	float CapsuleHalfHeight = AttackCapsuleHalfHeightData;
	
	// 트레이스 길이
	float TraceDistance = AttackTraceDistanceData;
	
	// 시작 위치 = 캐릭터 위치
	FVector Start = GetActorLocation();
	               
	// 끝 위치 = 캐릭터 앞 방향 * 거리
	FVector Forward = GetActorForwardVector();
	FVector End = Start + Forward * TraceDistance;
	
	// 충돌 파라미터 설정
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);   // 자기 자신 무시
	Params.bTraceComplex = false;
	Params.bReturnPhysicalMaterial = false;
	
	FHitResult Hit;
	
	// 시민은 마피아와 시민만 공격 가능.
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(MAFIA_OBJ);
	ObjectParams.AddObjectTypesToQuery(CITIZEN_OBJ);
	
	bool bHit = GetWorld()->SweepSingleByObjectType(Hit, Start, End, FQuat::Identity, ObjectParams, FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), Params);
	// 디버그: 캡슐 그리기

#if WITH_EDITOR
	DrawDebugCapsule(GetWorld(), (Start + End) * 0.5f, CapsuleHalfHeight, CapsuleRadius, FRotationMatrix::MakeFromZ(End - Start).ToQuat(), bHit ? FColor::Red : FColor::Green, false, 1.0f);
#endif

	if (bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *Hit.GetActor()->GetName());
		UGameplayStatics::ApplyDamage(Hit.GetActor(),AttackDamageData, GetController(),this, nullptr);
		
		AActor* HitActor = Hit.GetActor();
		CheckMafiaStat(HitActor);
	}
}

void AACCitizen::ExcuteMoneyCoolTime()
{
	if (IsValid(this) == false)
	{
		UE_LOG(LogHY, Error, TEXT("[MoneyCoolTimerHandle] this is Invalid"));	
		return;
	}
	//AC_LOG(LogHY, Error, TEXT("끝"));
}

void AACCitizen::UpdateAISkillFlag()
{
	if (IsValid(this) == false)
	{
		AC_LOG(LogHY, Error, TEXT("this는 Invalid입니다."));
		return;
	}
	
	// AC_LOG(LogHY, Error, TEXT("Call UpdateAISkillFlag"));
	
	bIsInitialSkillBlocked = false;
}

void AACCitizen::UpdateCharacterStatusFree()
{
	if (IsValid(this) == false)
	{
		AC_LOG(LogHY, Error, TEXT("this는 Invalid입니다."));
		return;
	}
	
	CharacterState = ECharacterState::Free;
	OnRep_CharacterState();
}

void AACCitizen::ChangeClothes()
{
	AACMainGameMode* MainGameMode = Cast<AACMainGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (MainGameMode == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("MainGameMode is nullptr"));
		return;
	}
	
	FOutfitCombo OutfitCombo =  MainGameMode->GetClothesFromPool();
	TopMesh = OutfitCombo.TopAsset.LoadSynchronous();
	BottomMesh = OutfitCombo.BottomAsset.LoadSynchronous();
	FaceMesh = OutfitCombo.FaceAsset.LoadSynchronous();
	FaceAccMesh = OutfitCombo.FaceAccAsset.LoadSynchronous();
	ShoesMesh = OutfitCombo.ShoesAsset.LoadSynchronous();
	HeadMesh = OutfitCombo.HairAsset.LoadSynchronous();
}

bool AACCitizen::DetectPolice()
{
	FVector Origin = GetActorLocation();
    
	//float CircleRadius = FMath::FRandRange(500.0f, 2000.0f);
	float CircleRadius = 500.0f;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel7);

	// 결과 배열
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(CircleRadius);

	// Sphere 범위 내 검사
	bool bFound = GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, ObjectQueryParams, Sphere);
	if (bFound == true)
	{
		// 0번째를 하는 게 맞는지
		PoliceCharacter = Cast<AACPoliceCharacter>(Overlaps[0].GetActor());
	}
	else
	{
		PoliceCharacter = nullptr;
	}
	
#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), Origin, CircleRadius, 16, bFound ? FColor::Red : FColor::Green, false, 1.0f);
#endif

	return bFound;
}

void AACCitizen::RunFromPolice()
{
	if (PoliceCharacter == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("PoliceCharacter is nullptr"));
		return;
	}
	
	// 경찰 위치와 내 위치를 구하고 방향 벡터를 구한다.
	FVector EnemyPosition = PoliceCharacter->GetActorLocation();
	FVector CurrentPosition = GetActorLocation();
	FVector DirVector = (CurrentPosition - EnemyPosition).GetSafeNormal2D();
	
	FVector NextPoint = CurrentPosition + DirVector * FMath::FRandRange(500.0f, 3000.0f);
	
	// NavMesh에서 Radius 500.f 범위 내 랜덤 포인트 구하기
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys == nullptr)
	{
		return;
	}
	
	// 결과 담을 변수
	FNavLocation RandomPoint;
	float CircleRadius = FMath::FRandRange(100.0f, 500.0f);
	bool bFound = NavSys->GetRandomReachablePointInRadius(NextPoint, CircleRadius, RandomPoint);
	if (bFound == false)
	{
		AC_LOG(LogHY, Error, TEXT("GetRandomReachablePointInRadius Fail: %s %s"), *NextPoint.ToString(), *RandomPoint.Location.ToString());
		return ;
	}
	
	// AC_LOG(LogHY, Log, TEXT("Enemy:%s"), *EnemyPosition.ToString());
	// AC_LOG(LogHY, Log, TEXT("My:%s"), *CurrentPosition.ToString());
	// AC_LOG(LogHY, Log, TEXT("Dir:%s"), *DirVector.ToString());
	// AC_LOG(LogHY, Log, TEXT("Next:%s"), *NextPoint.ToString());
	// AC_LOG(LogHY, Log, TEXT("Random Point: %s"), *RandomPoint.Location.ToString());
	
	AACCitizenAIController* AIController = Cast<AACCitizenAIController>(GetController());
	if (AIController == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("AIController is nullptr"));
		return;
	}
	UBlackboardComponent* BBComp = AIController->GetBlackboardComponent();
	if (BBComp == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("BBComp is nullptr"));
		return;
	}
	
	BBComp->SetValueAsVector(TEXT("RunPosition"), RandomPoint.Location);
}

void AACCitizen::JumpInPlace()
{
	Jump();
}

void AACCitizen::CheckMafiaStat(AActor* InActor)
{
	// 마피아인지 확인.
	AACMafiaCharacter* MafiaPawn = Cast<AACMafiaCharacter>(InActor);
	if (MafiaPawn == nullptr)
	{
		if (InActor == nullptr)
		{
			return;
		}
		//AC_LOG(LogHY, Error, TEXT("name: %s"), *InActor->GetName());
		return;	
	}
	
	float HP = MafiaPawn->GetCurrentHP();
	if (HP <= 0)
	{
		// 상태 Free로 변화.
		CharacterState = ECharacterState::Free;
		OnRep_CharacterState();
			
		MafiaCharacter = nullptr;
		
		AACCitizenAIController* AIController = Cast<AACCitizenAIController>(GetController());
		if (AIController == nullptr)
		{
			return;
		}
			
		UBlackboardComponent* BBComp = AIController->GetBlackboardComponent();
		if (BBComp == nullptr)
		{
			return;
		}
			
		BBComp->SetValueAsObject(TEXT("Target"), nullptr);
	}
}

#pragma region 매쉬 변경 시 호출되는 함수

void AACCitizen::ApplyHeadMesh()
{
	if (USkeletalMesh* SKMesh = HeadMesh.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before HeadMeshComp OK | prev:%s next:%s"), HeadMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *HeadMeshComp->GetSkeletalMeshAsset()->GetName(), *HeadMesh->GetName());
		HeadMeshComp->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After HeadMeshComp OK | prev:%s next:%s"), *HeadMeshComp->GetSkeletalMeshAsset()->GetName(), *HeadMesh->GetName());
	}
}

void AACCitizen::ApplyFaceMesh()
{
	if (USkeletalMesh* SKMesh = FaceMesh.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before FaceMeshComp OK | prev:%s next:%s"), FaceMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *FaceMeshComp->GetSkeletalMeshAsset()->GetName(), *FaceMesh->GetName());
		FaceMeshComp->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After FaceMeshComp OK | prev:%s next:%s"), *FaceMeshComp->GetSkeletalMeshAsset()->GetName(), *FaceMesh->GetName());
	}
}

void AACCitizen::ApplyTopMesh()
{
	if (USkeletalMesh* SKMesh = TopMesh.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before TopMeshComp OK | prev:%s next:%s"), TopMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *TopMeshComp->GetSkeletalMeshAsset()->GetName(), *TopMesh->GetName());
		TopMeshComp->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After TopMeshComp OK | prev:%s next:%s"), *TopMeshComp->GetSkeletalMeshAsset()->GetName(), *TopMesh->GetName());
	}
}

void AACCitizen::ApplyBottomMesh()
{
	if (USkeletalMesh* SKMesh = BottomMesh.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before BottomMeshComp OK | prev:%s next:%s"), BottomMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *BottomMeshComp->GetSkeletalMeshAsset()->GetName(), *BottomMesh->GetName());
		BottomMeshComp->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After BottomMeshComp OK | prev:%s next:%s"), *BottomMeshComp->GetSkeletalMeshAsset()->GetName(), *BottomMesh->GetName());
	}
}

void AACCitizen::ApplyShoesMesh()
{
	if (USkeletalMesh* SKMesh = ShoesMesh.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before ShoesMeshComp OK | prev:%s next:%s"), ShoesMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *ShoesMeshComp->GetSkeletalMeshAsset()->GetName(), *ShoesMesh->GetName());
		ShoesMeshComp->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After ShoesMeshComp OK | prev:%s next:%s"), *ShoesMeshComp->GetSkeletalMeshAsset()->GetName(), *ShoesMesh->GetName());
	}
}

void AACCitizen::ApplyFaceAccMesh()
{
	if (USkeletalMesh* SKMesh = FaceAccMesh.Get())
	{
		//AC_LOG(LogHY, Error, TEXT("Before FaceAccMeshComp OK | prev:%s next:%s"), FaceAccMeshComp->GetSkeletalMeshAsset() == nullptr ? TEXT("No Asset") : *FaceAccMeshComp->GetSkeletalMeshAsset()->GetName(), *FaceAccMesh->GetName());
		FaceAccMeshComp->SetSkeletalMesh(SKMesh);
		//AC_LOG(LogHY, Error, TEXT("After FaceAccMeshComp OK | prev:%s next:%s"), *FaceAccMeshComp->GetSkeletalMeshAsset()->GetName(), *FaceAccMesh->GetName());
	}
}

void AACCitizen::OnRep_HeadMesh()
{
	if (HeadMeshComp == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_HeadMesh: HeadMeshComp is NULL"));
		return; 
	}
	
	if (HeadMesh.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_HeadMesh: HeadMesh is NULL"));
		return;
	}

	if (HeadMesh.IsValid())
	{
		ApplyHeadMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다1"));	
	}
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(HeadMesh.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCitizen::ApplyHeadMesh));
}

void AACCitizen::OnRep_FaceMesh()
{
	if (FaceMeshComp == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_FaceMesh: FaceMeshComp is NULL"));
		return;
	}

	if (FaceMesh.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_FaceMesh: FaceMesh is NULL"));
		return;
	}

	if (FaceMesh.IsValid())
	{
		ApplyFaceMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다2"));	
	}
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(FaceMesh.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCitizen::ApplyFaceMesh));
}

void AACCitizen::OnRep_TopMesh()
{
	if (TopMeshComp == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_TopMesh: TopMeshComp is NULL"));
		return;
	}
	
	if (TopMesh.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_TopMesh: TopMesh is NULL"));
		return;
	}

	if (TopMesh.IsValid())
	{
		ApplyTopMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다3"));	
	}
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(TopMesh.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCitizen::ApplyTopMesh));
}

void AACCitizen::OnRep_BottomMesh()
{
	if (BottomMeshComp == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_BottomMesh: BottomMeshComp is NULL"));
		return; 
	}
	
	if (BottomMesh.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_BottomMesh: BottomMesh is NULL"));
		return;
	}

	if (BottomMesh.IsValid())
	{
		ApplyBottomMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다4"));	
	}
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(BottomMesh.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCitizen::ApplyBottomMesh));
}

void AACCitizen::OnRep_ShoesMesh()
{
	if (ShoesMeshComp == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_ShoesMesh: ShoesMeshComp is NULL"));
		return;
		
	}
	if (ShoesMesh.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_ShoesMesh: ShoesMesh is NULL"));
		return;
	}

	if (ShoesMesh.IsValid())
	{
		ApplyShoesMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다5"));	
	}
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(ShoesMesh.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCitizen::ApplyShoesMesh));
}

void AACCitizen::OnRep_FaceAccMesh()
{
	if (FaceAccMeshComp == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_FaceAccMesh: FaceAccMeshComp is NULL"));
		return;
	}
	
	if (FaceAccMesh.IsNull())
	{
		AC_LOG(LogHY, Warning, TEXT("OnRep_FaceAccMesh: FaceAccMesh is NULL"));
		return;
	}

	if (FaceAccMesh.IsValid())
	{
		ApplyFaceAccMesh();
		return;
	}
	else
	{
		AC_LOG(LogHY, Warning, TEXT("이거다6"));	
	}
	
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(FaceAccMesh.ToSoftObjectPath(),FStreamableDelegate::CreateUObject(this, &AACCitizen::ApplyFaceAccMesh));
}

void AACCitizen::OnRep_CharacterState()
{
	if (CharacterState == ECharacterState::OnDamage)
	{
		AC_LOG(LogHY, Warning, TEXT("Prev Damage WalSpeed Target: %f"), GetCharacterMovement()->MaxWalkSpeed);
		GetCharacterMovement()->MaxWalkSpeed = RunWalkSpeedData;
		AC_LOG(LogHY, Warning, TEXT("Cur  Damage WalSpeed Target: %f"), GetCharacterMovement()->MaxWalkSpeed);
	}
	else if (CharacterState == ECharacterState::Free)
	{
		AC_LOG(LogHY, Warning, TEXT("Prev Free WalSpeed Target: %f"), GetCharacterMovement()->MaxWalkSpeed);
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeedData;
		AC_LOG(LogHY, Warning, TEXT("Cur  Free WalSpeed Target: %f"), GetCharacterMovement()->MaxWalkSpeed);
	}
	else if (CharacterState == ECharacterState::Angry)
	{
		AC_LOG(LogHY, Warning, TEXT("Prev Angry WalSpeed Target: %f"), GetCharacterMovement()->MaxWalkSpeed);
		GetCharacterMovement()->MaxWalkSpeed = AngryWalkSpeedData;
		AC_LOG(LogHY, Warning, TEXT("Cur  Angry WalSpeed Target: %f"), GetCharacterMovement()->MaxWalkSpeed);
	}
}

void AACCitizen::RegenMoney()
{
	MoneyComp->GenerateRandomMoney(500);
}

void AACCitizen::TryRegenMoneyTimer()
{
	if (HasAuthority() == false)
	{
		AC_LOG(LogHY, Error, TEXT("AI인데 클라가 실행?"));
		return;
	}
	
	FTimerDelegate RegenDelegate;
	RegenDelegate.BindUObject(this, &AACCitizen::RegenMoney);
	float RegenRate = FMath::FRandRange(RegenRateMinData, RegenRateMaxData);
	GetWorld()->GetTimerManager().SetTimer(RegenMoneyTimerHandle, RegenDelegate, RegenRate, false);
}

void AACCitizen::MulticastPlayAttackMontage_Implementation()
{
	if (MeleeMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		AC_LOG(LogHY, Error, TEXT("Hi"));
		GetMesh()->GetAnimInstance()->Montage_Play(MeleeMontage, 1.0f);
	}
}

#pragma endregion

void AACCitizen::MulticastOnPlayMontage_Implementation(const FVector& Attack)
{
	FVector ForwardVector = GetActorForwardVector();
	FVector RightVector = GetActorRightVector();
	FVector DirVector = (Attack - GetActorLocation()).GetSafeNormal2D();
	float FrontBackResult = ForwardVector.Dot(DirVector);
	float RightLeftResult = RightVector.Dot(DirVector);
	
	GetMesh()->GetAnimInstance()->Montage_Play(DamagedMontage);
	if (FrontBackResult >= 0)
	{
		if (RightLeftResult >= 0)
		{
			// Front
			if (FrontBackResult >= RightLeftResult)
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Front"), DamagedMontage);
				// UE_LOG(LogTemp, Log, TEXT("Front - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
			else
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Right"), DamagedMontage);
				// UE_LOG(LogTemp, Log, TEXT("Right - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
		}
		else
		{
			// Front
			if (FrontBackResult >= abs(RightLeftResult))
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Front"), DamagedMontage);
				// UE_LOG(LogTemp, Log, TEXT("Front - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
			else
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Left"), DamagedMontage);
				// UE_LOG(LogTemp, Log, TEXT("Left - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
		}
	}
	else
	{
		if (RightLeftResult >= 0)
		{
			// Front
			if (abs(FrontBackResult) >= abs(RightLeftResult))
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Back"), DamagedMontage);
				// UE_LOG(LogTemp, Log, TEXT("Back - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
			else
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Right"), DamagedMontage);
				// UE_LOG(LogTemp, Log, TEXT("Right - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
		}
		else
		{
			// Front
			if (abs(FrontBackResult) >= abs(RightLeftResult))
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Back"), DamagedMontage);
				// UE_LOG(LogTemp, Log, TEXT("Back - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
			else
			{
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(TEXT("Left"), DamagedMontage);
				// UE_LOG(LogTemp, Log, TEXT("Left - F:%f, R:%f"), FrontBackResult, RightLeftResult);
			}
		}
		
	}
}



void AACCitizen::OnInteract(AACCharacter* ACPlayer, EInteractionKey InKey)
{
	if (ACPlayer == nullptr)
	{
		return;
	}

	AACMainPlayerController* PC = Cast<AACMainPlayerController>(ACPlayer->GetController());
	if (PC == nullptr)
	{
		AC_LOG(LogSY, Log, TEXT("PC is nullptr"));
		return;
	}

	//todo: DB로 교체
	switch (InKey)
	{
	case EInteractionKey::E:
		AC_LOG(LogSW, Log, TEXT("시민 신분증!"))
			PC->Client_ShowNotification(FText::FromString(TEXT("시민")), ENotificationType::None);
			break;
	case EInteractionKey::R:
		AC_LOG(LogSW, Log, TEXT("시민 사망!"))
		break;
	case EInteractionKey::T: break;

	}
}

EACInteractorType AACCitizen::GetInteractorType() const
{
	return EACInteractorType::Citizen;
}

UWidgetComponent* AACCitizen::GetInteractionWidget() const
{
	return InteractionWidgetComponent;
}

void AACCitizen::ShowInteractionHints(const TArray<UACInteractionData*>& Interactions)
{
	if (!InteractionWidgetComponent) return;

	// 위젯 생성 (최초 1회)
	if (!InteractionWidgetComponent->GetWidget() && InteractionInfoWidgetClass)
	{
		InteractionWidgetComponent->SetWidgetClass(InteractionInfoWidgetClass);
	}

	UACInteractionInfoWidget* HintWidget = Cast<UACInteractionInfoWidget>(
		InteractionWidgetComponent->GetWidget());
	if (!HintWidget)
	{
		return;
	}

	HintWidget->UpdateInteractions(Interactions);
	HintWidget->ShowWidget();
	InteractionWidgetComponent->SetVisibility(true);
}

void AACCitizen::HideInteractionHints()
{
	if (!InteractionWidgetComponent) return;

	UACInteractionInfoWidget* HintWidget = Cast<UACInteractionInfoWidget>(
		InteractionWidgetComponent->GetWidget());
	if (HintWidget)
		HintWidget->HideWidget();

	InteractionWidgetComponent->SetVisibility(false);
}

void AACCitizen::PlayHitEffect(float Duration)
{
	// Overlay Material이 없으면 리턴
	if (DamageOverlayMaterial == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AACCitizen: DamageOverlayMaterial is null! Please assign in Blueprint."));
		return;
	}

	// 모든 메시 컴포넌트 리스트 (ACCitizen은 *Comp 형태)
	TArray<USkeletalMeshComponent*> MeshComponents = {
		HeadMeshComp,
		FaceMeshComp,
		TopMeshComp,
		BottomMeshComp,
		ShoesMeshComp,
		FaceAccMeshComp,
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
		&AACCitizen::ResetHitEffect,
		Duration,
		false  // 한 번만 실행
	);
}

void AACCitizen::MulticastPlayHitEffect_Implementation(float Duration)
{
	PlayHitEffect(Duration);
}

void AACCitizen::ResetHitEffect()
{
	// 모든 메시 컴포넌트 리스트
	TArray<USkeletalMeshComponent*> MeshComponents = {
		HeadMeshComp,
		FaceMeshComp,
		TopMeshComp,
		BottomMeshComp,
		ShoesMeshComp,
		FaceAccMeshComp,
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
}