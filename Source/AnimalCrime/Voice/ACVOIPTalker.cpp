
#include "Voice/ACVOIPTalker.h"
#include "Components/AudioComponent.h"
#include "AnimalCrime.h"

void UACVOIPTalker::OnTalkingBegin(UAudioComponent* AudioComponent)
{
	Super::OnTalkingBegin(AudioComponent);

	CachedAudioComponent = AudioComponent;
	if (AudioComponent == nullptr)
	{
		return;
	}

	if (bIsMute == true)
	{
		AudioComponent->SetVolumeMultiplier(0.0f);
	}
	else
	{
		AudioComponent->SetVolumeMultiplier(VolumeMultiplier);
	}
}

void UACVOIPTalker::SetVOIPVolume(float NewVolumeMultiplier)
{
	VolumeMultiplier = NewVolumeMultiplier * 6.0f; //기본 소리가 작아 6.0f 곱하기
	if (CachedAudioComponent == nullptr)
	{
		return;
	}

	//볼륨을 조절
	if (bIsMute == true)
	{
		CachedAudioComponent->SetVolumeMultiplier(0.0f);
	}
	else
	{
		CachedAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
	}
}

void UACVOIPTalker::MuteToggle(bool bMute)
{
	bIsMute = bMute;
	if (CachedAudioComponent == nullptr)
	{
		return;
	}

	// 음소거 설정
	if (bIsMute == true)
	{
		CachedAudioComponent->SetVolumeMultiplier(0.0f);
	}
	else
	{
		CachedAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
	}
}
