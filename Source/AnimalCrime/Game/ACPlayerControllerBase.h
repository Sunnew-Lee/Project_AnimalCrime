#pragma once

#include "CoreMinimal.h"
#include "ACGameEnums.h"
#include "GameFramework/PlayerController.h"
#include "ACPlayerControllerBase.generated.h"

UCLASS()
class ANIMALCRIME_API AACPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()
public:
	AACPlayerControllerBase();
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
public:
	/**
		@brief 입력 모드 변경 (기본 입력 ↔ 설정 메뉴 입력)
		@param NewMode - 새로운 입력 모드
	**/
	virtual void ChangeInputMode(EInputMode NewMode);

public:
	/**
		@brief 소리 설정창 토글
		@param bVisible - true면 켜기, false면 끄기
	**/
	void SoundSettingToggle(bool bVisible);

public:
 /**
	 @brief 오디오 설정 적용
 **/
	UFUNCTION()
	void ApplyAudioSettings();

	// ===== 공통 입력 처리 핸들러 =====
protected:
	virtual void HandleMove(const struct FInputActionValue& Value);
	virtual void HandleLook(const struct FInputActionValue& Value);
	virtual void HandleJump(const struct FInputActionValue& Value);
	virtual void HandleStopJumping(const struct FInputActionValue& Value);
	virtual void HandleInteractStart(const struct FInputActionValue& Value);
	virtual void HandleInteractHold(const struct FInputActionValue& Value);
	virtual void HandleInteractRelease(const struct FInputActionValue& Value);
	virtual void HandleItemDrop(const struct FInputActionValue& Value);
	virtual void HandleAttack(const struct FInputActionValue& Value);
	virtual void HandleSprintStart(const struct FInputActionValue& Value);
	virtual void HandleSprintEnd(const struct FInputActionValue& Value);
	virtual void HandleSettingsClose(const struct FInputActionValue& Value);
	virtual void HandleSoundSetting(const struct FInputActionValue& Value);

protected:
	virtual bool CanUseSkill() const;

public:
	UFUNCTION(Client, Reliable)
	void Client_CleanupVoiceBeforeTravel();

	// ===== 공통 입력 관련 =====
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> InteractAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> ItemDropAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> MeleeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> SprintAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> SettingsMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> SettingsCloseAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> SoundSettingAction;

	// ===== 오디오 설정 관련 =====
protected:
	FTimerHandle AudioSettingsTimer;

	// ===== UI =====
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UACSoundSetting> SoundSettingWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UACSoundSetting> SoundSettingWidget;
};
