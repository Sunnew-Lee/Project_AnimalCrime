#include "ACSoundSetting.h"
#include "Components/ComboBoxString.h"
#include "Engine/Engine.h"
#include "Game/ACAdvancedFriendsGameInstance.h"
#include "AnimalCrime.h"

void UACSoundSetting::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogSY, Log, TEXT("오디오 UI 생성"));

	if (OutputDeviceComboBox)
	{
		OutputDeviceComboBox->OnSelectionChanged.AddDynamic(
			this, &UACSoundSetting::OnOutputDeviceChanged);
	}
}

void UACSoundSetting::RefreshAudioDevices()
{
	UE_LOG(LogSY, Log, TEXT("[SoundSetting] RefreshAudioDevices"));

	InitInputDeviceComboBox();
	InitOutputDeviceComboBox();
}


void UACSoundSetting::InitInputDeviceComboBox()
{
	if (!InputDeviceComboBox)
		return;

	InputDeviceComboBox->ClearOptions();
	InputDeviceComboBox->AddOption(TEXT("Use System Default Microphone"));
	InputDeviceComboBox->SetSelectedIndex(0);

	UE_LOG(LogSY, Log, TEXT("입력 디바이스 초기화"));
}

void UACSoundSetting::InitOutputDeviceComboBox()
{
	if (!OutputDeviceComboBox)
		return;

	bUpdatingOutputDeviceList = true;

	OutputDeviceComboBox->ClearOptions();
	CachedOutputDevices.Empty();

	FOnAudioOutputDevicesObtained Delegate;
	Delegate.BindUFunction(this, FName("OnOutputDevicesObtained"));

	UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(
		this,
		Delegate
	);

	UE_LOG(LogSY, Log, TEXT("출력 디바이스 초기화"));
}
void UACSoundSetting::OnOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& Devices)
{
	CachedOutputDevices = Devices;

	FString SavedDeviceId;
	UACAdvancedFriendsGameInstance* GI = GetWorld()->GetGameInstance<UACAdvancedFriendsGameInstance>();
	if (GI == nullptr)
	{
		return;
	}
	SavedDeviceId = GI->SelectedAudioOutputDeviceId;

	for (const FAudioOutputDeviceInfo& Device : Devices)
	{
		OutputDeviceComboBox->AddOption(Device.Name);

		if (!SavedDeviceId.IsEmpty() && Device.DeviceId == SavedDeviceId)
		{
			OutputDeviceComboBox->SetSelectedOption(Device.Name);
		}
	}

	// 저장된 게 없거나 못 찾았을 때 
	if (OutputDeviceComboBox->GetSelectedIndex() == INDEX_NONE)
	{
		for (const FAudioOutputDeviceInfo& Device : Devices)
		{
			if (Device.bIsCurrentDevice || Device.bIsSystemDefault)
			{
				OutputDeviceComboBox->SetSelectedOption(Device.Name);
				break;
			}
		}
	}

	bUpdatingOutputDeviceList = false;

}

void UACSoundSetting::OnOutputDeviceChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	if (bUpdatingOutputDeviceList)
	{
		UE_LOG(LogSY, Log, TEXT("UI 갱신 중 선택 이벤트 무시"));
		return;
	}

	UE_LOG(LogSY, Log, TEXT("[SoundSetting] Output device selection changed: %s"), *SelectedItem);

	for (const FAudioOutputDeviceInfo& Device : CachedOutputDevices)
	{
		if (Device.Name == SelectedItem)
		{
			UE_LOG(LogSY, Log,
				TEXT("[SoundSetting] Matched device | Name: %s | DeviceId: %s"),
				*Device.Name,
				*Device.DeviceId
			);

			// 델리게이트 생성 및 바인딩
			FOnCompletedDeviceSwap OnSwapCompleted;
			OnSwapCompleted.BindUFunction(this, FName("OnDeviceSwapCompleted"));

			// 디바이스 변경 실행
			UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(
				this,
				Device.DeviceId,
				OnSwapCompleted
			);

			break;
		}
	}
}

void UACSoundSetting::OnDeviceSwapCompleted(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Success)
	{
		UE_LOG(LogSY, Log,
			TEXT("[SoundSetting] Audio output device swap successful! Current Device: %s"),
			*SwapResult.CurrentDeviceId
		);
		if (UACAdvancedFriendsGameInstance* GI = GetWorld()->GetGameInstance<UACAdvancedFriendsGameInstance>())
		{
			GI->SelectedAudioOutputDeviceId = SwapResult.CurrentDeviceId;
		}
	}
	else
	{
		UE_LOG(LogSY, Error,
			TEXT("[SoundSetting] Audio output device swap failed! Requested: %s, Current: %s"),
			*SwapResult.RequestedDeviceId,
			*SwapResult.CurrentDeviceId
		);
	}
}