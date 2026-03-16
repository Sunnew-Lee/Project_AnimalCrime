
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

	UFUNCTION()
	void OnSliderValueChanged(float Value);

	UFUNCTION()
	void OnSliderCaptureEnd();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> PlayerNameText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> PlayerAvatarImage;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USlider> PlayerSlider;

protected:
	UPROPERTY()
	TObjectPtr<class UACPlayerVoiceControlEntryData> ItemData;
};
