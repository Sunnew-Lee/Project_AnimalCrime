// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ACPoliceCharacter.h"
#include "AnimalCrime.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Component/ACMoneyComponent.h"
#include <Kismet/GameplayStatics.h>

#include "Component/ACShopComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Item/ACItemData.h"

AACPoliceCharacter::AACPoliceCharacter()
{
	// 콜리전 세팅
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("PoliceCollision"));
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> HeadMeshRef(TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Hat_057.SK_Hat_057"));
	if (HeadMeshRef.Succeeded() == true)
	{
		HeadMesh->SetSkeletalMesh(HeadMeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FaceAccMeshRef(TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Moustache_002.SK_Moustache_002"));
	if (FaceAccMeshRef.Succeeded() == true)
	{
		FaceAccMesh->SetSkeletalMesh(FaceAccMeshRef.Object);
	}
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FaceMeshRef(TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Male_emotion_angry_003.SK_Male_emotion_angry_003"));
	if (FaceMeshRef.Succeeded() == true)
	{
		FaceMesh->SetSkeletalMesh(FaceMeshRef.Object);
	}

	TopMesh->SetSkeletalMesh(nullptr);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BottomMeshRef(TEXT("/Game/Creative_Characters_FREE/Skeleton_Meshes/SK_Costume_10_001.SK_Costume_10_001"));
	if (BottomMeshRef.Succeeded() == true)
	{
		BottomMesh->SetSkeletalMesh(BottomMeshRef.Object);
	}

	ShoesMesh->SetSkeletalMesh(nullptr);
	
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;  // 경찰

	//경찰은 시작부터 무전기 소유
	VoiceGroup = EVoiceGroup::Police;
}

void AACPoliceCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AACPoliceCharacter::PostNetInit()
{
	Super::PostNetInit();
}

void AACPoliceCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)
	{
		AC_LOG(LogHY, Error, TEXT("HasAuthority is false"));
		return;
	}
	
	// Police의 type의 돈을 설정.
	MoneyComp->InitMoneyComponent(EMoneyType::MoneyPoliceType);
	
	// 월급 주기 지정 및 월급 실행 함수
	ChangeSalary(20.0f);
}

float AACPoliceCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// 감옥 상태에서는 맞아서는 안됌.
	if (CharacterState == ECharacterState::Stun)
	{
		return 0.0;
	}
	if (CharacterState == ECharacterState::OnDamage ||
		CharacterState == ECharacterState::OnInteract)
	{
		AC_LOG(LogHY, Error, TEXT("나와서는 안됩니다."));
		return 0.0;
	}
	
	if (DamageCauser == nullptr)
	{
		return 0.0f;
	}
	
	if (DamageAmount < 5.0f)
	{
		return 0.0f;	
	}
	
	float SuperResult = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	// 권한있는 APawn만 계산해야 함.
	if (HasAuthority() == false)
	{
		return 0.0f;
	}

	if (DamageAmount > 0.0f)
	{
		// todo: 경찰 Stun 효과 시간
		MulticastPlayHitEffect(OnHitEffectRate);  // 모든 클라이언트에 전파
	}

	AC_LOG(LogHY, Error, TEXT("Damage:%f"), DamageAmount);
	AC_LOG(LogHY, Error, TEXT("Name:%s"), *DamageCauser->GetName());
	SetCharacterState(ECharacterState::Stun);
	
	// todo: 경찰 기절 5초 타이머
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AACPoliceCharacter::UpdateCharacterStatusRevive);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, StunRate, false);
	
	return SuperResult;
}

void AACPoliceCharacter::MulticastPlayHitEffect_Implementation(float Duration)
{
	PlayHitEffect(Duration);
}

EACCharacterType AACPoliceCharacter::GetCharacterType() const
{
	return EACCharacterType::Police;
}

void AACPoliceCharacter::AttackHitCheck(int32 DamageAmount)
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

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(DESTROYABLE_OBJ);
	ObjectParams.AddObjectTypesToQuery(MAFIA_OBJ);
	ObjectParams.AddObjectTypesToQuery(CITIZEN_OBJ);

	bool bHit = GetWorld()->SweepSingleByObjectType(Hit, Start, End, FQuat::Identity, ObjectParams, FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), Params);

	// 디버그: 캡슐 그리기
#if WITH_EDITOR
	DrawDebugCapsule(GetWorld(), (Start + End) * 0.5f, CapsuleHalfHeight, CapsuleRadius, FRotationMatrix::MakeFromZ(End - Start).ToQuat(), bHit ? FColor::Red : FColor::Green, false, 1.0f);
#endif

	if (bHit)
	{
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

void AACPoliceCharacter::UpdateCharacterStatusRevive()
{
	if (IsValid(this) == false)
	{
		AC_LOG(LogHY, Error, TEXT("this가 올바르지 않습니다."));
		return;
	}
	AC_LOG(LogHY, Error, TEXT("상태가 변경되었습니다."));
	bStun = false;
	SetCharacterState(ECharacterState::Free);
}

void AACPoliceCharacter::CalculateSalary()
{
	if (IsValid(this) == false)
	{
		AC_LOG(LogHY, Error, TEXT("this is not Valid"));
		return;
	}
	
	// 돈 추가 로직
	MoneyComp->EarnMoney(100);
}

void AACPoliceCharacter::ChangeSalary(float InTimeRate)
{
	GetWorldTimerManager().ClearTimer(SalaryTimerHandle);
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AACPoliceCharacter::CalculateSalary);
	TimeRate = InTimeRate;
	GetWorld()->GetTimerManager().SetTimer(SalaryTimerHandle, TimerDelegate, TimeRate, true);
}

void AACPoliceCharacter::OnInteract(AACCharacter* ACPlayer, EInteractionKey InKey)
{
}

EACInteractorType AACPoliceCharacter::GetInteractorType() const
{
	return EACInteractorType::Police;
}

