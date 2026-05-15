
#pragma once

#include "CoreMinimal.h"
#include "Net/VoiceConfig.h"
#include "ACVOIPTalker.generated.h"

UCLASS()
class ANIMALCRIME_API UACVOIPTalker : public UVOIPTalker
{
	GENERATED_BODY()

protected:
    /**
        @brief 음성 입력 시작 시 호출되는 함수
        @param AudioComponent - 음성 입력을 처리하는 오디오 컴포넌트
    **/
    virtual void OnTalkingBegin(UAudioComponent* AudioComponent) override;

public:
    UFUNCTION(BlueprintCallable, Category = "VOIP")
    void SetVOIPVolume(float NewVolumeMultiplier);

    UFUNCTION(BlueprintCallable, Category = "VOIP")
	void MuteToggle(bool bMute);

private:
    UPROPERTY()
    TObjectPtr<UAudioComponent> CachedAudioComponent;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VOIP")
    float VolumeMultiplier = 3.0f;

	//!< 음소거 여부, true면 음소거, false면 음소거 해제
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VOIP")
	uint8 bIsMute : 1 = false; 
};
