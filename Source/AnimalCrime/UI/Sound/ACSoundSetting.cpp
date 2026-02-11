#include "ACSoundSetting.h"
#include "Components/ComboBoxString.h"
#include "Engine/Engine.h"
#include "Game/ACAdvancedFriendsGameInstance.h"
#include "AnimalCrime.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

void UACSoundSetting::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogSY, Log, TEXT("오디오 UI 생성"));

	if (InputDeviceComboBox != nullptr)
	{
		InputDeviceComboBox->OnSelectionChanged.AddDynamic(this, &UACSoundSetting::OnInputDeviceChanged);
	}

	if (OutputDeviceComboBox != nullptr)
	{
		OutputDeviceComboBox->OnSelectionChanged.AddDynamic(this, &UACSoundSetting::OnOutputDeviceChanged);
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
	if (InputDeviceComboBox == nullptr)
	{
		return;
	}

	InputDeviceComboBox->ClearOptions();
	CachedInputDevices.Empty();

#if PLATFORM_WINDOWS
	// Windows 오디오 디바이스 열거
	IMMDeviceEnumerator* pEnumerator = nullptr;
	IMMDeviceCollection* pCollection = nullptr;

	CoInitialize(nullptr);
	HRESULT hr = CoCreateInstance(
		__uuidof(MMDeviceEnumerator),
		nullptr,
		CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator),
		(void**)&pEnumerator
	);

	if (SUCCEEDED(hr))
	{
		hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);

		if (SUCCEEDED(hr))
		{
			UINT count;
			pCollection->GetCount(&count);

			UE_LOG(LogSY, Log, TEXT("[SoundSetting] Found %d input devices"), count);

			for (UINT i = 0; i < count; i++)
			{
				IMMDevice* pDevice = nullptr;
				hr = pCollection->Item(i, &pDevice);

				if (SUCCEEDED(hr))
				{
					// 디바이스 ID 가져오기
					LPWSTR pwszID = nullptr;
					pDevice->GetId(&pwszID);
					FString DeviceId(pwszID);
					CoTaskMemFree(pwszID);

					// 디바이스 이름 가져오기
					IPropertyStore* pProps = nullptr;
					pDevice->OpenPropertyStore(STGM_READ, &pProps);

					PROPVARIANT varName;
					PropVariantInit(&varName);
					pProps->GetValue(PKEY_Device_FriendlyName, &varName);
					FString DeviceName(varName.pwszVal);
					PropVariantClear(&varName);

					// 기본 디바이스 확인
					IMMDevice* pDefaultDevice = nullptr;
					bool bIsDefault = false;
					if (SUCCEEDED(pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDefaultDevice)))
					{
						LPWSTR pwszDefaultID = nullptr;
						pDefaultDevice->GetId(&pwszDefaultID);
						bIsDefault = (DeviceId == FString(pwszDefaultID));
						CoTaskMemFree(pwszDefaultID);
						pDefaultDevice->Release();
					}

					// 캐시에 추가
					FInputDeviceInfo DeviceInfo;
					DeviceInfo.DeviceId = DeviceId;
					DeviceInfo.DeviceName = DeviceName;
					DeviceInfo.bIsDefault = bIsDefault;
					CachedInputDevices.Add(DeviceInfo);

					InputDeviceComboBox->AddOption(DeviceName);

					UE_LOG(LogSY, Log, TEXT("[SoundSetting] Input Device: %s (Default: %s)"),
						*DeviceName, bIsDefault ? TEXT("Yes") : TEXT("No"));

					pProps->Release();
					pDevice->Release();
				}
			}

			pCollection->Release();
		}

		pEnumerator->Release();
	}

	CoUninitialize();

	// 저장된 디바이스 선택
	UACAdvancedFriendsGameInstance* GI = GetWorld()->GetGameInstance<UACAdvancedFriendsGameInstance>();
	if (GI && !GI->SelectedAudioInputDeviceId.IsEmpty())
	{
		for (int32 i = 0; i < CachedInputDevices.Num(); i++)
		{
			if (CachedInputDevices[i].DeviceId == GI->SelectedAudioInputDeviceId)
			{
				InputDeviceComboBox->SetSelectedIndex(i);
				break;
			}
		}
	}

	// 선택된 게 없으면 기본 디바이스 선택
	if (InputDeviceComboBox->GetSelectedIndex() == INDEX_NONE)
	{
		for (int32 i = 0; i < CachedInputDevices.Num(); i++)
		{
			if (CachedInputDevices[i].bIsDefault)
			{
				InputDeviceComboBox->SetSelectedIndex(i);
				break;
			}
		}
	}

#else
	// 다른 플랫폼
	InputDeviceComboBox->AddOption(TEXT("Use System Default Microphone"));
	InputDeviceComboBox->SetSelectedIndex(0);
#endif

	UE_LOG(LogSY, Log, TEXT("입력 디바이스 초기화 완료"));

}

void UACSoundSetting::InitOutputDeviceComboBox()
{
	if (OutputDeviceComboBox == nullptr)
	{
		return;
	}

	OutputDeviceComboBox->ClearOptions();
	CachedOutputDevices.Empty();

	FOnAudioOutputDevicesObtained Delegate;
	Delegate.BindUFunction(this, FName("OnOutputDevicesObtained"));

	UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(
		this,
		Delegate
	);

	UE_LOG(LogSY, Log, TEXT("출력 디바이스 초기화 완료"));
}

void UACSoundSetting::OnInputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UE_LOG(LogSY, Log, TEXT("[SoundSetting] Input device selection changed: %s"), *SelectedItem);

	int32 SelectedIndex = InputDeviceComboBox->GetSelectedIndex();
	if (CachedInputDevices.IsValidIndex(SelectedIndex))
	{
		const FInputDeviceInfo& Device = CachedInputDevices[SelectedIndex];

		UE_LOG(LogSY, Log,
			TEXT("[SoundSetting] Setting input device | Name: %s | ID: %s"),
			*Device.DeviceName,
			*Device.DeviceId
		);

		// 게임 인스턴스에 저장
		UACAdvancedFriendsGameInstance* GI = GetWorld()->GetGameInstance<UACAdvancedFriendsGameInstance>();
		if (GI)
		{
			GI->SelectedAudioInputDeviceId = Device.DeviceId;
			UE_LOG(LogSY, Log, TEXT("[SoundSetting] Input device ID saved"));
		}

		// 스팀 보이스챗에 디바이스 ID 전달
		// 스팀 API에서 지원하는 경우에만 작동
		// 대부분의 경우 스팀은 시스템 기본 마이크를 사용하므로
		// Windows 설정에서 기본 마이크를 변경해야 할 수 있습니다
	}
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
	SavedDeviceId = GI->SelectedAudioOutputDeviceId; // 게임 인스턴스에서 저장된 디바이스 ID 가져오기
	UE_LOG(LogSY, Log, TEXT("[SoundSetting] SavedDeviceId: %s"), *SavedDeviceId);

	// 콤보박스에 디바이스 목록 추가 및 저장된 디바이스 선택
	for (const FAudioOutputDeviceInfo& Device : Devices)
	{
		OutputDeviceComboBox->AddOption(Device.Name);

		if (SavedDeviceId.IsEmpty() == false && Device.DeviceId == SavedDeviceId)
		{
			OutputDeviceComboBox->SetSelectedOption(Device.Name);
		}
		else
		{
			UE_LOG(LogSY, Log, TEXT("[SoundSetting] Device matching fail: %s DeviceId: %s"), *Device.Name, *Device.DeviceId);
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
}

void UACSoundSetting::OnOutputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
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
		UACAdvancedFriendsGameInstance* GI = GetWorld()->GetGameInstance<UACAdvancedFriendsGameInstance>();
		if (GI == nullptr)
		{
			return;
		}
		GI->SelectedAudioOutputDeviceId = SwapResult.RequestedDeviceId;
		UE_LOG(LogSY, Log, TEXT("[SoundSetting] Audio output device swap successful! RequestedDeviceId: %s"), *SwapResult.RequestedDeviceId);
	}
	else
	{
		UE_LOG(LogSY, Error, TEXT("[SoundSetting] Audio output device swap failed! Requested: %s, Current: %s"), *SwapResult.RequestedDeviceId, *SwapResult.CurrentDeviceId);
	}
}