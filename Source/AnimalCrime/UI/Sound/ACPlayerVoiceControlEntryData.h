#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ACPlayerVoiceControlEntryData.generated.h"

UCLASS()
class ANIMALCRIME_API UACPlayerVoiceControlEntryData : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TObjectPtr<class AACPlayerState> PlayerState;
};
