#pragma once

#include "CoreMinimal.h"
#include "Game/ACGameEnums.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interface/ACInteractInterface.h"
#include "Net/VoiceConfig.h"
#include "ACCharacter.generated.h"

class AACMainPlayerController;


DECLARE_MULTICAST_DELEGATE_OneParam(FOnSprintChanged, int32 /*Current*/);
DECLARE_MULTICAST_DELEGATE(FOnSprintUIShow);
DECLARE_MULTICAST_DELEGATE(FOnSprintUIHide);




UCLASS()
class ANIMALCRIME_API AACCharacter : public ACharacter, public IACInteractInterface
{
	GENERATED_BODY()

public:
	AACCharacter();
	/**
		@brief  캐릭터 정보를 반환하는 함수. 캐릭터 베이스는 시민.
		@retval  - 캐릭터 정보 Enum
	**/
	virtual EACCharacterType GetCharacterType() const;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	/**
		@brief 현재 입력매핑 컨텍스트를 전부 지우고 새로운 입력매핑 컨텍스트로 바꾸는 함수
		@param NewMode - 입력모드 Enum
	**/
	void ChangeInputMode(EInputMode NewMode);

	// ===== 입력 핸들러 (PlayerController가 호출) =====
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	virtual void InteractStarted(int32 InputIndex = 0);  // 기본값 0 (E 키)
	virtual void InteractHolding(const float DeltaTime);
	virtual void InteractReleased();

	virtual void ItemDrop();
	virtual void Attack();
	//virtual void SettingsClose(const FInputActionValue& Value);
	virtual void Jump() override;
	virtual void SettingsClose();
	virtual void SetSoundSetting();

public:
	/**
	 * @brief PlayerController로 부터 Dash 키가 눌렸는지 Input을 받는 함수
	 * @param Value 장치로 부터 얻은 Input 값
	 */
	void Dash(const FInputActionValue& Value);

	// 서버에 Dash 요청 함수
	UFUNCTION(Server, Reliable)
	void ServerDash();

	// 쿨타임 시 Flag 되돌리는 함수.
	void ResetDashFlag();


	// 클라이언트 Input을 받는 함수
	void Sprint(const FInputActionValue& Value);

	// 서버에 Dash 요청 함수
	UFUNCTION(Server, Reliable)
	void ServerSprintStart();

	// 서버에 Dash 요청 함수
	UFUNCTION(Server, Reliable)
	void ServerSprintEnd();

	AACMainPlayerController* GetMainPlayerController() const;

protected:

	UFUNCTION(Server, Reliable)
	void ServerInteract(AActor* Target, EInteractionKey InKey);

	UFUNCTION(Server, Reliable)
	virtual void ServerItemDrop();

	/**
		@brief 상호작용 시작 시 캐릭터 상태 변경
		@param Target - 상호작용 당하는 액터
	**/
	UFUNCTION(Server, Reliable)
	void ServerFreezeCharacter(AActor* Target);

	/**
		@brief 상호작용 초기화 시 캐릭터 상태 변경
		@param Target - 상호작용 당하는 액터
	**/
	UFUNCTION(Server, Reliable)
	void ServerUnfreezeCharacter(AActor* Target);

	// === 홀드 상호작용 RPC ===

 /**
	 @brief 상호작용 시작 시 몽타주 재생을 위한 함수
	 @param TargetActor     - 상호작용 당하는 액터
	 @param InteractionData - 상호작용 정보
 **/
	UFUNCTION(Server, Reliable)
	void ServerStartHoldInteraction(AActor* TargetActor, class UACInteractionData* InteractionData);

	/**
		@brief 상호작용 초기화 시 몽타주 중단을 위한 함수
		@param TargetActor     - 상호작용 당하는 액터
	**/
	UFUNCTION(Server, Reliable)
	void ServerStopHoldInteraction(AActor* TargetActor);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartHoldInteraction(
		AActor* TargetActor,
		UAnimMontage* InitiatorMontage,
		UAnimMontage* TargetMontage,
		bool bDoRotateTarget = false
	);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopHoldInteraction(AActor* TargetActor);

public:

	UFUNCTION(BlueprintCallable)
	bool CheckProcessAttack() const;
	UFUNCTION(BlueprintCallable)
	void ChangeAttackTrue();
	UFUNCTION(BlueprintCallable)
	void ChangeAttackFalse();


public:
	TObjectPtr<class USkeletalMeshComponent> GetHeadMesh() const { return HeadMesh; }
	TObjectPtr<class USkeletalMeshComponent> GetFaceMesh() const { return FaceMesh; }
	TObjectPtr<class USkeletalMeshComponent> GetFaceAccMesh() const { return FaceAccMesh; }
	TObjectPtr<class USkeletalMeshComponent> GetTopMesh() const { return TopMesh; }
	TObjectPtr<class USkeletalMeshComponent> GetBottomMesh() const { return BottomMesh; }
	TObjectPtr<class USkeletalMeshComponent> GetShoesMesh() const { return ShoesMesh; }


protected:
	//!< 메쉬 컴포넌트
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	// TObjectPtr<class USkeletalMeshComponent> HeadMesh;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	// TObjectPtr<class USkeletalMeshComponent> FaceMesh;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	// TObjectPtr<class USkeletalMeshComponent> TopMesh;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	// TObjectPtr<class USkeletalMeshComponent> BottomMesh;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	// TObjectPtr<class USkeletalMeshComponent> ShoesMesh;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	// TObjectPtr<class USkeletalMeshComponent> FaceAccMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USkeletalMeshComponent> HeadMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USkeletalMeshComponent> FaceMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USkeletalMeshComponent> TopMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USkeletalMeshComponent> BottomMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USkeletalMeshComponent> ShoesMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USkeletalMeshComponent> FaceAccMesh;

	// UPROPERTY(ReplicatedUsing = OnRep_HeadMesh)
	// TObjectPtr<class USkeletalMesh> HeadMeshReal;
	//
	// UPROPERTY(ReplicatedUsing = OnRep_FaceMesh)
	// TObjectPtr<class USkeletalMesh> FaceMeshReal;
	//
	// UPROPERTY(ReplicatedUsing = OnRep_TopMesh)
	// TObjectPtr<class USkeletalMesh> TopMeshReal;
	//
	// UPROPERTY(ReplicatedUsing = OnRep_BottomMesh)
	// TObjectPtr<class USkeletalMesh> BottomMeshReal;
	//
	// UPROPERTY(ReplicatedUsing = OnRep_ShoesMesh)
	// TObjectPtr<class USkeletalMesh> ShoesMeshReal;
	//
	// UPROPERTY(ReplicatedUsing = OnRep_FaceAccMesh)
	// TObjectPtr<class USkeletalMesh> FaceAccMeshReal;
	
	UPROPERTY(ReplicatedUsing=OnRep_HeadMesh)
	TSoftObjectPtr<class USkeletalMesh> HeadMeshReal;

	UPROPERTY(ReplicatedUsing=OnRep_FaceMesh)
	TSoftObjectPtr<class USkeletalMesh> FaceMeshReal;

	UPROPERTY(ReplicatedUsing=OnRep_TopMesh)
	TSoftObjectPtr<class USkeletalMesh> TopMeshReal;

	UPROPERTY(ReplicatedUsing=OnRep_BottomMesh)
	TSoftObjectPtr<class USkeletalMesh> BottomMeshReal;

	UPROPERTY(ReplicatedUsing=OnRep_ShoesMesh)
	TSoftObjectPtr<class USkeletalMesh> ShoesMeshReal;

	UPROPERTY(ReplicatedUsing=OnRep_FaceAccMesh)
	TSoftObjectPtr<class USkeletalMesh> FaceAccMeshReal;

#pragma region 매시 Get/Set
public:
	FORCEINLINE USkeletalMesh* GetHeadMeshT()		const { return HeadMeshReal.Get(); }
	FORCEINLINE USkeletalMesh* GetFaceMeshT()		const { return FaceMeshReal.Get(); }
	FORCEINLINE USkeletalMesh* GetTopMeshT()			const { return TopMeshReal.Get(); }
	FORCEINLINE USkeletalMesh* GetBottomMeshT()		const { return BottomMeshReal.Get(); }
	FORCEINLINE USkeletalMesh* GetShoesMeshT()		const { return ShoesMeshReal.Get(); }
	FORCEINLINE USkeletalMesh* GetFaceAccMeshT()		const { return FaceAccMeshReal.Get(); }

	FORCEINLINE void UpdateHeadMesh()		const { HeadMesh->SetSkeletalMesh(HeadMeshReal.Get()); }
	FORCEINLINE void UpdateFaceMesh()		const { FaceMesh->SetSkeletalMesh(FaceMeshReal.Get()); }
	FORCEINLINE void UpdateTopMesh()		const { TopMesh->SetSkeletalMesh(TopMeshReal.Get()); }
	FORCEINLINE void UpdateBottomMesh()		const { BottomMesh->SetSkeletalMesh(BottomMeshReal.Get()); }
	FORCEINLINE void UpdateShoesMesh()		const { ShoesMesh->SetSkeletalMesh(ShoesMeshReal.Get()); }
	FORCEINLINE void UpdateFaceAccMesh()	const { FaceAccMesh->SetSkeletalMesh(FaceAccMeshReal.Get()); }

	FORCEINLINE void SetHeadMesh(USkeletalMesh* InMesh) { HeadMeshReal = InMesh; }
	FORCEINLINE void SetFaceMesh(USkeletalMesh* InMesh) { FaceMeshReal = InMesh; }
	FORCEINLINE void SetTopMesh(USkeletalMesh* InMesh) { TopMeshReal = InMesh; }
	FORCEINLINE void SetBottomMesh(USkeletalMesh* InMesh) { BottomMeshReal = InMesh; }
	FORCEINLINE void SetShoesMesh(USkeletalMesh* InMesh) { ShoesMeshReal = InMesh; }
	FORCEINLINE void SetFaceAccMesh(USkeletalMesh* InMesh) { FaceAccMeshReal = InMesh; }
#pragma endregion
public:
	UFUNCTION()
	void OnRep_HeadMesh();

	UFUNCTION()
	void OnRep_FaceMesh();

	UFUNCTION()
	void OnRep_TopMesh();

	UFUNCTION()
	void OnRep_BottomMesh();

	UFUNCTION()
	void OnRep_ShoesMesh();

	UFUNCTION()
	void OnRep_FaceAccMesh();
	
	
	void ApplyHeadMesh();
	void ApplyFaceMesh();
	void ApplyTopMesh();
	void ApplyBottomMesh();
	void ApplyShoesMesh();
	void ApplyFaceAccMesh();

public:
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	UCameraComponent* GetGunCamera() const { return GunCamera; }

protected:
	//!< 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> GunSpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> GunCamera;

protected:
	/** 몽타주: 기본 공격  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Member|Attack|Anim", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> MeleeMontage;

	/** 몽타주: 총  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Member|Attack|Anim", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> ZoomMontage;
	/** 몽타주: 기본 공격  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Member|Attack|Anim", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> ShootMontage;

	/** 플레그: 공격 시도 중 여부 */
	UPROPERTY()
	uint8 bAttackFlag : 1 = false;

	UFUNCTION(Server, Reliable)
	void ServerAttack();
	void PerformAttackTrace();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Member|Attack|Anim", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> EscapeMontage;

	/** 플레그: 공격 시도 중 여부 */
	UPROPERTY()
	uint8 bEscapeFlag : 1 = false;

	// UFUNCTION(Server, Reliable)
	// void ServerEscape();
	// void PerformEscape();

public:
	virtual void AttackHitCheck(int32 DamageAmount = 1);
	bool IsHoldingGun();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttackMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayZoomMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStopAllMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayShootMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayEscapeSkillMontage();


	void FireHitscan();

	/**
		@brief 캐릭터의 Carry 상태를 설정하는 진입 함수.
		   호출한 주체가 서버인지 클라이언트인지에 따라 내부적으로 Server RPC 또는 Multicast RPC를 호출하여 모든 플레이어에게 애니메이션이 동기화되도록 처리한다.
	**/
	UFUNCTION(BlueprintCallable)
	void SetCarryState(bool bPlay);

	/**
		@brief 클라이언트가 서버에게 Carry 상태 변경을 요청하기 위한 RPC 함수.
	**/
	UFUNCTION(Server, Reliable)
	void Server_SetCarryState(bool bPlay);

	/**
		@brief 서버에서 호출되어 모든 클라이언트에게 Carry 상태 변경을 전파하는 RPC 함수.
	**/
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCarryState(bool bPlay);

	UFUNCTION(BlueprintCallable)
	void SetZoomInState(bool bPlay);

	UFUNCTION(Server, Reliable)
	void Server_SetZoomInState(bool bPlay);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetZoomInState(bool bPlay);

	/**
		   @brief brief VoiceGroup이 리플리케이트될 때 호출되는 함수.
			   로컬 플레이어의 무전기 상태가 변경되면 모든 캐릭터의 VOIP 설정을 업데이트하고,
			   다른 캐릭터의 무전기 상태가 변경되면 해당 캐릭터의 VOIP 설정만 업데이트한다.
	**/
	UFUNCTION()
	void OnRep_VoiceGroup();

public:
	/**
		@brief 무전기 보유 상태를 설정하는 함수.
		@param bNewHasRadio - true면 무전기 보유, false면 미보유
	**/
	//UFUNCTION(BlueprintCallable, Category = "VOIP")
	//void SetHasRadio(bool bNewHasRadio);

	//UFUNCTION(BlueprintCallable, Category = "VOIP")
	//bool GetHasRadio() const { return bHasRadio; }

	/**
		@brief 무전기 보유 상태를 설정하는 함수.
		@param NewVoiceGroup - 새로운 무전기 그룹 Enum
	**/
	UFUNCTION(BlueprintCallable, Category = "VOIP")
	void SetVoiceGroup(EVoiceGroup NewVoiceGroup);

	/**
		@brief 로컬 플레이어의 무전기 상태에 따라 모든 다른 캐릭터들의 VOIPTalker Attenuation을 업데이트하는 함수.
			양쪽 다 무전기가 있으면 Attenuation을 제거하여 거리에 상관없이 음성이 들리게 한다.
	**/
	void UpdateRadioVoiceSettings();

	/**
		@brief 다른 캐릭터의 VOIPTalker Attenuation 설정 (로컬에서만 호출)
		@param bUseRadio - true면 라디오, false면 거리 기반
	**/
	void SetVOIPMode(bool bUseRadio);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<class UACMoneyComponent> MoneyComp;

	//!< 상호작용 함수
public:
	/**
		@brief NearInteractables 배열에 상호작용 가능한(상호작용 컴포넌트를 지닌) 액터 저장. 상호작용 컴포넌트와 오버랩 시작시 불림.
		@param Interactor - 배열에 추가할 상호작용 가능한 액터
	**/
	void AddInteractable(AActor* Interactor);
	/**
		@brief NearInteractables 배열에 저장되어있는 액터 제거. 상호작용 컴포넌트와 오버랩 끝날시 불림.
		@param Interactor - 배열에서 제거할 상호작용 가능한 액터
	**/
	void RemoveInteractable(AActor* Interactor);

	//!< 상호작용 인터페이스 구현
protected:
	virtual void OnInteract(AACCharacter* ACPlayer, EInteractionKey InKey) override;
	virtual EACInteractorType GetInteractorType() const override;

	//=== NEW: Widget interface overrides ===
	virtual class UWidgetComponent* GetInteractionWidget() const override;
	virtual void ShowInteractionHints(const TArray<class UACInteractionData*>& Interactions) override;
	virtual void HideInteractionHints() override;

private:
	/**
		@brief  NearInteractables Array의 Actor들을 플레이어와 거리가 가까운 순서로 Sort. Sort 여부를 반환.
		@retval  - NearInteractables가 Sort되었으면 true, 아니면 false 반환
	**/
	bool SortNearInteractables();

	/**
		@brief NearInteractables에서 가장 가까운 유효한 액터를 Focus로 설정
		@details Overlap 이벤트 발생 시(Add/RemoveInteractable) 호출됨
	**/
	void UpdateFocus();

	/**
		@brief 홀드 상호작용 관련 멤버변수를 모두 초기화
	**/
	void ResetHoldInteract();

	/**
		@brief  홀드 상호작용의 UI에 사용될 진행도
		@retval  - 진행도를 0.0~1.0 사이의 값으로 반환
	**/
	float GetHoldProgress() const;

	//!< 상호작용 멤버변수
protected:
	/**
		@brief VOIPTalker 등록 시도 함수. 타이머로 주기적으로 호출되어 VOIPTalker가 생성될 때까지 시도함.
		자기자신은 호출하지않음. 원격 플레이어만 호출해야함.
	**/
	void TryRegisterVOIPTalker();

public:
	// VOIPTalker 정리 함수
	void CleanupVOIPTalker();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	TObjectPtr<class UACInteractableComponent> InteractBoxComponent;

	//=== NEW: Interaction Widget ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact")
	TObjectPtr<class UWidgetComponent> InteractionWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	TSubclassOf<class UACInteractionInfoWidget> InteractionInfoWidgetClass;

private:
	TArray<AActor*> NearInteractables;

	//=== NEW: Focus System ===
	UPROPERTY()
	AActor* FocusedInteractable = nullptr;

	TArray<class UACInteractionData*> FocusedInteractions;

	EInteractionKey CurrentHoldKey;
	AActor* CurrentHoldTarget;
	float CurrentHoldTime = 0.f;
	float RequiredHoldTime = 0.f;
	bool bIsHoldingInteract = false;

	// 현재 상호작용 몽타주 저장
	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentInteractionMontage;

	// 타겟의 상호작용 몽타주 저장
	UPROPERTY()
	TObjectPtr<UAnimMontage> TargetInteractionMontage;

	// ===== 상점 관련 =====
protected:
	// 상점 컴포넌트는 유지 (아이템 장착 로직이 있으므로)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
	TObjectPtr<class UACShopComponent> ShopComponent;



	UFUNCTION()
	void OnRep_CharacterState();
public:
	ECharacterState GetCharacterState() const;
	ECharacterState GetPrevCharacterState() const;
	void SetCharacterState(ECharacterState InCharacterState);
protected:
	ESettingMode SettingMode = ESettingMode::None;

	FTimerHandle TimerHandle;


	UPROPERTY(ReplicatedUsing = OnRep_CharacterState, EditAnywhere, BlueprintReadWrite, Category = "State")
	ECharacterState CharacterState;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "State")
	ECharacterState PrevCharacterState;
public:
	void SetFreeState();
	void SetOnDamageState();
	void SetStunState();
	void SetInteractState();
	void SetOnInteractState();

protected:
	float CalculateMoveSpeed() const;

protected: // Dash 전용 맴버 변수
	FTimerHandle DashTimerHandle;

	UPROPERTY(Replicated)
	uint8 bDashCoolDown = true;

public:
	FOnSprintChanged OnSprintChanged;

	FOnSprintUIShow OnSprintUIShow;
	FOnSprintUIHide OnSprintUIHide;
protected: // Sprint 전용 맴버 변수
	UPROPERTY(ReplicatedUsing = OnRep_Sprint)
	uint8 bSprint : 1 = false;

	UFUNCTION()
	void OnRep_Sprint();

	UFUNCTION()
	void GaugeUp();

	UFUNCTION()
	void GaugeDown();

	FTimerHandle SprintGaugeDownTimerHandle;
	FTimerHandle SprintGaugeUpTimerHandle;

	UFUNCTION()
	void OnRep_SprintGauge();

	UPROPERTY(ReplicatedUsing = OnRep_SprintGauge)
	int32 SprintGauge = 10;

public:
	bool CanZoomIn();

public:
	UFUNCTION(Server, Unreliable)
	void ServerPlayZoomIn();

	UFUNCTION(Server, Unreliable)
	void ServerPlayZoomOut();

protected:
	UPROPERTY(Replicated, EditAnywhere)
	TObjectPtr<class UACDestroyableStatComponent> Stat;

// 사운드 관련
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> BatSwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> GunSound;

// 피격 효과 관련
public:
	/**
		 @brief 피격 효과 (빨간색 Overlay 적용)
		 @param Duration - 효과 지속 시간 (초)
	 **/
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void PlayHitEffect(float Duration = 0.2f);
protected:
	/**
		@brief 피격 효과 종료 (Overlay 제거)
	**/
	void ResetHitEffect();

	// 피격 효과 Material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TObjectPtr<UMaterialInterface> DamageOverlayMaterial;

	// 스턴 효과 Material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
	TObjectPtr<UMaterialInterface> StunOverlayMaterial;

	UFUNCTION()
	void OnRep_Stun();
private:
	// 피격 효과 타이머
	FTimerHandle HitEffectTimerHandle;

	void ApplyStunOverlay();
	void RemoveStunOverlay();

// 나중에 사라져야하는 코드들.	
public:
	UFUNCTION(Server, Reliable)
	void ServerShoot();
	
	UFUNCTION(Server, Reliable)
	void ServerPlayShootMontage();
	
public:
	int32 GetBulletCount() const;
	void AddBullets(int32 InBulletCount);
	void ClearBullets();
	void SpendBullets(int32 InBulletCount);

	UFUNCTION()
	void OnRep_BulletCount();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_BulletCount, EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	int32 BulletCount = 0;
	// VOIP 관련
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VOIP")
	TObjectPtr<class UACVOIPTalker> VOIPTalker;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VOIP")
	TObjectPtr<class USoundAttenuation> VoiceProximity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VOIP")
	TObjectPtr<class USoundAttenuation> VoiceRadio;

	FTimerHandle VOIPTalkerTimerHandle;

public:
	//!< 무전기 그룹, 양쪽 다 같은 무전기 그룹에 있으면 거리에 상관없이 음성이 들림.
	UPROPERTY(ReplicatedUsing = OnRep_VoiceGroup, BlueprintReadWrite, Category = "VOIP")
	EVoiceGroup VoiceGroup = EVoiceGroup::None;
	
	
protected:	// 캐릭터 스킬의 맴버 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<class UACSkillData> SkillDataAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintMoveSpeedData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OriginMafiaMoveSpeedData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OriginPoliceMoveSpeedData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintGaugeData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintGaugeUpData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DashForwardImpulseData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DashUpwardImpulseData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DashCoolTimeData;

	// 나중에 추가
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OriginZVelocity = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StunWalkSpeed = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StunZVelocity = 0.0f;

	
	FTimerHandle EscapeTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHpData = 5.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GunDamage = 5.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeaponDamage = 3.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NormalDamage = 1.0f;
	
	
	float BaseRunSpeed;
	
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	uint8 bOnDamage:1;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	uint8 bStun:1;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	uint8 bInteract:1;
	
	UPROPERTY(Replicated,EditAnywhere, BlueprintReadWrite)
	uint8 bOnInteract:1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsCarry : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsZoomIn : 1;
};
