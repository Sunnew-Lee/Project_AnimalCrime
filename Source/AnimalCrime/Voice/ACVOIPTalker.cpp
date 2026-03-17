
#include "Voice/ACVOIPTalker.h"
#include "Components/AudioComponent.h"

void UACVOIPTalker::OnTalkingBegin(UAudioComponent* AudioComponent)
{
	Super::OnTalkingBegin(AudioComponent);

	CachedAudioComponent = AudioComponent;
	if (AudioComponent == nullptr)
	{
		return;
	}
	AudioComponent->SetVolumeMultiplier(VolumeMultiplier);
}

void UACVOIPTalker::SetVOIPVolume(float NewVolumeMultiplier)
{
	VolumeMultiplier = NewVolumeMultiplier * 6.0f; //기본 소리가 작아 6.0f 곱하기
	if (CachedAudioComponent == nullptr)
	{
		return;
	}
	CachedAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
}
