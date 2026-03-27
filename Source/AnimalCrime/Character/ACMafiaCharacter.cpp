
#include "Character/ACMafiaCharacter.h"
#include "Item/ACEscapeMissionBomb.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "ACBlackMarketDealer.h"
#include "EscapeQuest/ACBombInstallArea.h"
#include "EscapeQuest/ACEscapeArea.h"
#include "Kismet/GameplayStatics.h"

#include "Game/ACMainGameState.h"
#include "Game/ACMainGameMode.h"
#include "AnimalCrime.h"
#include "Component/ACMoneyComponent.h"
#include "Components/CapsuleComponent.h"
#include "Component/ACDestroyableStatComponent.h"
#include "Component/ACShopComponent.h"
#include "DSP/LFO.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Game/ACMainPlayerController.h"
#include "Item/ACItemData.h"
#include "Sound/SoundBase.h"
#include "UI/ACHUDWidget.h"

#include "EscapeQuest/ACDestructibleBuilding.h"
#include "Component/ACInteractableComponent.h"

AACMafiaCharacter::AACMafiaCharacter()
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("MafiaCollision"));

	Stat = CreateDefaultSubobject<UACDestroyableStatComponent>(TEXT("StatComponent"));

	VoiceGroup = EVoiceGroup::None;

	InteractBoxComponent->SetMargin(FVector(100.f));
}

void AACMafiaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AACMafiaCharacter, HandBomb);
	DOREPLIFETIME(AACMafiaCharacter, Constrband);
	DOREPLIFETIME(AACMafiaCharacter, bHasWalkyTalky);
}

void AACMafiaCharacter::BeginPlay()
{
	AC_LOG(LogHY, Error, TEXT("Begin"));
	Super::BeginPlay();

	// 서버만 GameState에 등록
	if (HasAuthority() == false)
	{
		AC_LOG(LogHY, Error, TEXT("HasAuthority is false"));
		return;
	}

	AACMainGameMode* ACGameMode = GetWorld()->GetAuthGameMode<AACMainGameMode>();
	// 게임 모드 확인
	if (ACGameMode == nullptr)
	{
		AC_LOG(LogHY, Warning, TEXT("ACGameMode is null"));
		return;
	}

	/*
	*	랜덤 옷 입히기 로직
	*/
	FOutfitCombo OutFit = ACGameMode->GetClothesFromPool();

	HeadMeshReal = OutFit.HairAsset.LoadSynchronous();
	OnRep_HeadMesh();
	FaceMeshReal = OutFit.FaceAsset.LoadSynchronous();
	OnRep_FaceMesh();
	TopMeshReal = OutFit.TopAsset.LoadSynchronous();
	OnRep_TopMesh();
	BottomMeshReal = OutFit.BottomAsset.LoadSynchronous();
	OnRep_BottomMesh();
	ShoesMeshReal = OutFit.ShoesAsset.LoadSynchronous();
	OnRep_ShoesMesh();
	FaceAccMeshReal = OutFit.FaceAccAsset.LoadSynchronous();
	OnRep_FaceAccMesh();

	// Mafia는 100원으로 시작
	MoneyComp->InitMoneyComponent(EMoneyType::MoneyMafiaType);

	// GameState에 등록
	AACMainGameState* ACGameState = GetWorld()->GetGameState<AACMainGameState>();
	if (ACGameState == nullptr)
	{
		return;
	}
	ACGameState->MafiaPlayers.Add(this);
	AC_LOG(LogSY, Warning, TEXT("Mafia:: %d"), ACGameState->MafiaPlayers.Num());

	// 마피아가 처음에 가지고 있는 돈 설정
	MoneyComp->InitMoneyComponent(EMoneyType::MoneyMafiaType);
	AC_LOG(LogHY, Warning, TEXT("Before HP=%f | Authority=%d"),
		Stat->GetCurrentHp(),
		HasAuthority());

	// 체력 설정.
	Stat->SetMaxHp(5);
	Stat->SetCurrentHp(5);
	Stat->SetArmor(0);
	AC_LOG(LogHY, Warning, TEXT("After HP=%f | Authority=%d"), Stat->GetCurrentHp(), HasAuthority());

	// Tax 설정
	ChangeTax(60);

	AC_LOG(LogHY, Error, TEXT("End"));
}

void AACMafiaCharacter::PostNetInit()
{
	Super::PostNetInit();
}

void AACMafiaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/*TickDeltaTime -=DeltaSeconds;
	if (TickDeltaTime <= 0.0f)
	{
		AC_LOG(LogHY, Error, TEXT("My HP is %f"), Stat->GetCurrentHp());
		TickDeltaTime += 1.0f;
	}*/
}

void AACMafiaCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AC_LOG(LogHY, Warning, TEXT("AACMafiaCharacter::EndPlay"));
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

float AACMafiaCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 감옥 상태에서는 맞아서는 안됌.
	if (CharacterState == ECharacterState::OnDamage ||
		CharacterState == ECharacterState::Stun ||
		CharacterState == ECharacterState::Prison ||
		CharacterState == ECharacterState::PrisonEscape ||
		CharacterState == ECharacterState::OnInteract)
	{
		return 0.0;
	}

	float SuperResult = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 피격 사운드 재생
	if (HitSound && DamageAmount > 0.0f)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	// 권한있는 APawn만 계산해야 함.
	if (HasAuthority() == false)
	{
		return 0.0f;
	}

	// 서버에서만 실행되는 영역에 Multicast 호출 
	if (DamageAmount > 0.0f)
	{
		// todo: 마피아 OnDamage 효과 시간
		MulticastPlayHitEffect(DamaeRate);  // 모든 클라이언트에 전파
	}

	AC_LOG(LogHY, Error, TEXT("Damage:%f"), DamageAmount);
	if (EventInstigator != nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("MyName:%s EventInstigator:%s"), *GetName(), *EventInstigator->GetName());
	}
	if (DamageCauser != nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("MyName:%s DamageCauser:%s"), *GetName(), *DamageCauser->GetName());
	}

	float CurrentHp = Stat->GetCurrentHp();
	CurrentHp -= DamageAmount;
	Stat->SetCurrentHp(CurrentHp);
	//AC_LOG(LogHY, Error, TEXT("My HP is %f"), Stat->GetCurrentHp());

	FTimerDelegate TimerDelegate;
	if (CurrentHp <= 0)
	{
		// 상태 변경
		SetCharacterState(ECharacterState::Stun);
		if (CharacterState == ECharacterState::Stun)
		{
			// todo: 마피아 기절 타이머
			TimerDelegate.BindUObject(this, &AACMafiaCharacter::UpdateCharacterStatusRevive);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, StunRate, false);
		}
	}
	else
	{
		if (DamageAmount > 0.0f)
		{
			// todo: 마피아 OnDamage 효과 시간
			MulticastPlayHitEffect(2.f);  // 모든 클라이언트에 전파
		}

		SetCharacterState(ECharacterState::OnDamage);
		if (CharacterState == ECharacterState::OnDamage)
		{
			// todo: 마피아 OnDamage 타이머
			TimerDelegate.BindUObject(this, &AACMafiaCharacter::UpdateCharacterStatusFree);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, DamaeRate, false);
		}
	}

	return DamageAmount;
}

// OnDamage -> Free
void AACMafiaCharacter::UpdateCharacterStatusFree()
{
	if (IsValid(this) == false)
	{
		AC_LOG(LogHY, Error, TEXT("this가 올바르지 않습니다."));
		return;
	}

	AC_LOG(LogHY, Error, TEXT("상태가 변경되었습니다."));
	// false로 미리 바꿔야 계산이 된다.
	bOnDamage = false;
	SetCharacterState(ECharacterState::Free);
}

// Stun -> Free
void AACMafiaCharacter::UpdateCharacterStatusRevive()
{
	if (IsValid(this) == false)
	{
		AC_LOG(LogHY, Error, TEXT("this가 올바르지 않습니다."));
		return;
	}
	AC_LOG(LogHY, Error, TEXT("상태가 변경되었습니다."));
	//bStun = false;
	SetCharacterState(ECharacterState::Free);
	if (HasAuthority() == true)
	{
		Stat->SetCurrentHp(MaxHpData);
	}
}

void AACMafiaCharacter::OnInteract(AACCharacter* ACPlayer, EInteractionKey InKey)
{
	if (ACPlayer == nullptr)
	{
		return;
	}

	//// 경찰과 상호작용(신분증)
	//if (this->CharacterState == ECharacterState::Free)
	//{
	//	AC_LOG(LogSW, Log, TEXT("마피아 신분증!"));
	//}

	AACMainPlayerController* PC = Cast<AACMainPlayerController>(ACPlayer->GetController());
	if (PC == nullptr)
	{
		AC_LOG(LogSY, Log, TEXT("PC is nullptr"));
		return;
	}

	//todo: DB로 교체
	AACMainGameMode* GM = GetWorld()->GetAuthGameMode<AACMainGameMode>();
	switch (InKey)
	{
	case EInteractionKey::E:
		AC_LOG(LogSW, Log, TEXT("마피아 신분증!"))
			PC->Client_ShowNotification(FText::FromString(TEXT("마피아")), ENotificationType::None);
		break;
	case EInteractionKey::R:
		// 경찰과 상호작용(투옥)
		//if (this->CharacterState == ECharacterState::Stun)
		//{
		//	
		//}
		// 경찰과 상호작용(투옥)
		//AACMainGameMode* GM = GetWorld()->GetAuthGameMode<AACMainGameMode>();
		if (GM == nullptr)
		{
			return;
		}
		GM->ImprisonCharacter(this);  // GameMode에 캡슐화 함수 사용

		break;
	case EInteractionKey::T: break;

	}

}

EACInteractorType AACMafiaCharacter::GetInteractorType() const
{
	return EACInteractorType::Mafia;
}

void AACMafiaCharacter::MulticastPlayHitEffect_Implementation(float Duration)
{
	PlayHitEffect(Duration);
}

EACCharacterType AACMafiaCharacter::GetCharacterType() const
{
	return EACCharacterType::Mafia;
}

void AACMafiaCharacter::ItemDrop()
{
	AC_LOG(LogSY, Log, TEXT("ItemDrop Key!!"));
	ServerItemDrop(); //서버에 알림.
}

void AACMafiaCharacter::ServerItemDrop_Implementation()
{
	if (HandBomb != nullptr)
	{
		//캐릭터에서 폭탄 분리
		HandBomb->DetachFromCharacter();

		//참조 해제
		HandBomb->AttachedCharacter = nullptr;

		// Impulse 적용은 서버가 직접 해야 하므로 여기서 처리
		//UStaticMeshComponent* MeshComp = HandBomb->GetBombMeshComp();
		//if (MeshComp)
		//{
		//    FVector ThrowDir = GetActorForwardVector() + FVector(0, 0, 0.3f);
		//    MeshComp->AddImpulse(ThrowDir * 300.f, NAME_None, true);
		//}

		HandBomb = nullptr;

		//폭탄 설치 가능 구역 숨기기

		ClientSetBombHeld(false);
		AC_LOG(LogSY, Log, TEXT("Bomb dropped"));
	}
	else
	{
		AC_LOG(LogSY, Warning, TEXT("No bomb to drop"));
	}
}

void AACMafiaCharacter::ClientSetBombHeld_Implementation(bool bIsHolding)
{
	AACMainGameState* GS = GetWorld()->GetGameState<AACMainGameState>();
	if (GS == nullptr)
	{
		return;
	}
	AC_LOG(LogSY, Log, TEXT("%d"), GS->BombAreas.Num());
	for (AACBombInstallArea* Area : GS->BombAreas)
	{
		if (Area == nullptr)
		{
			continue;
		}

		Area->SetActorHiddenInGame(!bIsHolding);

		AC_LOG(LogSY, Log, TEXT("ClientSetBombAreaVisible"));
	}

	AACMainPlayerController* PC = Cast<AACMainPlayerController>(GetController());
	if (PC == nullptr || PC->ACHUDWidget == nullptr)
	{
		return;
	}
	bIsHolding ? PC->ACHUDWidget->ShowDropUI() : PC->ACHUDWidget->HideDropUI();

	// === Building Fade 제어 (이 클라이언트에서만 보임!) ===
	TArray<AActor*> FoundBuildings;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AACDestructibleBuilding::StaticClass(), FoundBuildings);

	for (AActor* Actor : FoundBuildings)
	{
		AACDestructibleBuilding* Building = Cast<AACDestructibleBuilding>(Actor);
		if (Building && Building->BombSpot == ESpot::Bank)
		{
			if (bIsHolding == true)
			{
				Building->StartFadeLoop();  // 이 클라이언트에서만 fade 시작
			}
			else
			{
				Building->StopFade();  // 이 클라이언트에서만 fade 중지
			}
		}
	}
}

void AACMafiaCharacter::ClientSetEscapeAreaVisible_Implementation(bool bVisible)
{
	AACMainGameState* GS = GetWorld()->GetGameState<AACMainGameState>();
	if (GS == nullptr)
	{
		return;
	}
	AC_LOG(LogSY, Log, TEXT("%d"), GS->EscapeAreas.Num());
	for (AACEscapeArea* Area : GS->EscapeAreas)
	{
		if (Area == nullptr)
		{
			continue;
		}
		Area->SetActorHiddenInGame(!bVisible);

		AC_LOG(LogSY, Log, TEXT("ClientSetBombAreaVisible"));
	}
}

void AACMafiaCharacter::OnRep_HandBomb()
{
	if (HandBomb)
	{
		AC_LOG(LogSY, Log, TEXT("OnRep_HandBomb: Now holding bomb"));
	}
	else
	{
		AC_LOG(LogSY, Log, TEXT("OnRep_HandBomb: No longer holding bomb"));
	}
}

void AACMafiaCharacter::AttackHitCheck(int32 DamageAmount)
{
	// AACCharacter 클래스(Empty)
	Super::AttackHitCheck(DamageAmount);

	// FireHitscan();
	// 캡슐 크기
	float CapsuleRadius = 30.0f;
	float CapsuleHalfHeight = 60.0f;

	// 트레이스 길이
	float TraceDistance = 200.0f;

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

	// bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_GameTraceChannel2 | ECC_GameTraceChannel4, FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), Params);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(DESTROYABLE_OBJ);
	ObjectParams.AddObjectTypesToQuery(MAFIA_OBJ);
	ObjectParams.AddObjectTypesToQuery(POLICE_OBJ);
	ObjectParams.AddObjectTypesToQuery(CITIZEN_OBJ);

	bool bHit = GetWorld()->SweepSingleByObjectType(Hit, Start, End, FQuat::Identity, ObjectParams, FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), Params);

	// 디버그: 캡슐 그리기
#if WITH_EDITOR
	DrawDebugCapsule(GetWorld(), (Start + End) * 0.5f, CapsuleHalfHeight, CapsuleRadius, FRotationMatrix::MakeFromZ(End - Start).ToQuat(), bHit ? FColor::Red : FColor::Green, false, 1.0f);
#endif

	if (bHit)
	{
		// @Todo: 리펙토링 필요. 급해서 이렇게 짬.
		if (DamageAmount >= 5.0f)
		{
			AC_LOG(LogHY, Log, TEXT("Damage Amount %d"), DamageAmount);
			UGameplayStatics::ApplyDamage(Hit.GetActor(), DamageAmount, GetController(), this, nullptr);
			return;
		}
		AC_LOG(LogHY, Warning, TEXT("Hit Actor: %s"), *Hit.GetActor()->GetName());
		UACItemData* EquippedWeapon = ShopComponent->EquippedWeapon;
		if (EquippedWeapon == nullptr)
		{
			AC_LOG(LogHY, Log, TEXT("EquippedWeapon is nullptr Damage %f"), NormalDamage);
			UGameplayStatics::ApplyDamage(Hit.GetActor(), NormalDamage, GetController(), this, nullptr);
			return;
		}
		if (EquippedWeapon->ItemType == EItemType::Weapon)
		{
			AC_LOG(LogHY, Log, TEXT("EquippedWeapon is 빠따 Damage %f"), WeaponDamage);
			UGameplayStatics::ApplyDamage(Hit.GetActor(), WeaponDamage, GetController(), this, nullptr);
			return;
		}
	}
}

void AACMafiaCharacter::CalculateTax()
{
	if (IsValid(this) == false)
	{
		AC_LOG(LogHY, Error, TEXT("this is not Valid"));
		return;
	}

	// 돈 추가 로직
	MoneyComp->LoseMoney(5);
}

void AACMafiaCharacter::ChangeTax(float InTimeRate)
{
	GetWorldTimerManager().ClearTimer(TaxTimerHandle);

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AACMafiaCharacter::CalculateTax);
	TaxTimeRate = InTimeRate;
	GetWorld()->GetTimerManager().SetTimer(TaxTimerHandle, TimerDelegate, TaxTimeRate, true);
}

void AACMafiaCharacter::AddContraband()
{
	++Constrband;

	// 로컬 PlayerController에 알림
	OnRep_Contraband();
}

void AACMafiaCharacter::SubtractContraband()
{
	--Constrband;
	if (Constrband < 0)
	{
		Constrband = 0;
	}

	// 로컬 PlayerController에 알림
	OnRep_Contraband();
}

void AACMafiaCharacter::SetWalkyTalky(bool bInHasWalkyTalky)
{
	bHasWalkyTalky = bInHasWalkyTalky;

	// 로컬 PlayerController에 알림
	OnRep_HasWalkyTalky();
}

void AACMafiaCharacter::MoveToEscapeWaitingLocation()
{
	// 임의로 정한 탈출 대기 장소
	SetActorLocation(FVector(-6050.0f, 100.0f, 100.0f));
}

float AACMafiaCharacter::GetCurrentHP() const
{
	return Stat->GetCurrentHp();
}

void AACMafiaCharacter::OnRep_Contraband()
{
	// PlayerController에 알림
	AACMainPlayerController* PC = GetController<AACMainPlayerController>();
	if (PC != nullptr)
	{
		PC->SetHasContraband(Constrband > 0);
	}
}

void AACMafiaCharacter::OnRep_HasWalkyTalky()
{
	// PlayerController에 알림
	AACMainPlayerController* PC = GetController<AACMainPlayerController>();
	if (PC != nullptr)
	{
		PC->SetHasWalkyTalky(bHasWalkyTalky);
	}
}

void AACMafiaCharacter::ExcuteEscape()
{

	if (EscapeCount <= 0)
	{
		return;
	}

	ServerEscape();
}

void AACMafiaCharacter::ServerEscape_Implementation()
{
	EscapeCount -= 1;
	PerformEscape();
}

void AACMafiaCharacter::PerformEscape()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return;
	}

	MulticastPlayEscapeSkillMontage();
}
