
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
};
