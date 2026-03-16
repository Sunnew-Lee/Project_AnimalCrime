#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AudioMixerBlueprintLibrary.h"
#include "ACSoundSetting.generated.h"

UCLASS()
class ANIMALCRIME_API UACSoundSetting : public UUserWidget
{
	GENERATED_BODY()

protected:
	struct FInputDeviceInfo
	{
		FString DeviceId;
		FString DeviceName;
		bool bIsDefault;
	};

protected:
	virtual void NativeConstruct() override;

public:
 /**
     @brief 오디오 UI 업데이트 함수
 **/
	UFUNCTION()
	void RefreshAudioDevices();

protected:
	// 입출력 디바이스 콤보박스 초기화 함수
	void InitInputDeviceComboBox();
	void InitOutputDeviceComboBox();

	// 플레이어별 음성 세팅 업데이트 함수
	void UpdatePlayerVoiceList();

	UFUNCTION()
	void OnInputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnOutputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	// delegate callback
	UFUNCTION()
	void OnOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& Devices);

	UFUNCTION()
	void OnDeviceSwapCompleted(const FSwapAudioOutputResult& SwapResult);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UComboBoxString> InputDeviceComboBox;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UComboBoxString> OutputDeviceComboBox;

	// 디바이스 캐시
	TArray<FInputDeviceInfo> CachedInputDevices;
	TArray<FAudioOutputDeviceInfo> CachedOutputDevices;

	UPROPERTY()
	FOnAudioOutputDevicesObtained OutputDeviceDelegate;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UListView> PlayerVoiceList;
};
