
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "ACPlayerVoiceControlEntry.generated.h"


UCLASS()
class ANIMALCRIME_API UACPlayerVoiceControlEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void UpdatePlayer(class AACPlayerState* InPlayerState);

	/**
		@brief	볼륨슬라이더의 값이 변경될 때 호출되는 함수
		@param Value - 변경된 볼륨 값 (0.0 ~ 1.0)
	**/
	UFUNCTION()
	void OnSliderValueChanged(float Value);

	/**
		@brief	슬라이더조작이 끝났을 때 호출되는 함수. 볼륨 변경을 게임인스턴스에 적용하는 데 사용됨.
	**/
	UFUNCTION()
	void OnSliderCaptureEnd();

	/**
		@brief 음소거 체크박스의 상태가 변경될 때 호출되는 함수
		@param bIsChecked - 체크박스가 체크되면 플레이어의 음성이 음소거되고, 체크 해제되면 음성이 다시 활성화됨.
	**/
	UFUNCTION()
	void OnMuteCheckBoxChanged(bool bIsChecked);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> PlayerNameText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> PlayerAvatarImage;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USlider> PlayerSlider;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> VolumeText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCheckBox> MuteCheckBox;

protected:
	UPROPERTY()
	TObjectPtr<class UACPlayerVoiceControlEntryData> ItemData;
};
