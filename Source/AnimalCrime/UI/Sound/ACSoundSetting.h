#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AudioMixerBlueprintLibrary.h"
#include "ACSoundSetting.generated.h"

class UComboBoxString;

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
	UFUNCTION()
	void RefreshAudioDevices();

protected:
	void InitInputDeviceComboBox();
	void InitOutputDeviceComboBox();

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
	TObjectPtr<UComboBoxString> InputDeviceComboBox;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> OutputDeviceComboBox;

	// 디바이스 캐시
	TArray<FInputDeviceInfo> CachedInputDevices;
	TArray<FAudioOutputDeviceInfo> CachedOutputDevices;

	UPROPERTY()
	FOnAudioOutputDevicesObtained OutputDeviceDelegate;
};
