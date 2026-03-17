
#include "UI/Sound/ACPlayerVoiceControlEntry.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "AdvancedSteamFriendsLibrary.h"
#include "AdvancedFriendsLibrary.h"
#include "Game/ACAdvancedFriendsGameInstance.h"
#include "Game/ACPlayerState.h"
#include "Engine/World.h"
#include "ACPlayerVoiceControlEntryData.h"
#include "Character/ACCharacter.h"
#include "Voice/ACVOIPTalker.h"
#include "AnimalCrime.h"

void UACPlayerVoiceControlEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	ItemData = Cast<UACPlayerVoiceControlEntryData>(ListItemObject);
	if (ItemData == nullptr)
	{
		return;
	}

	UpdatePlayer(ItemData->PlayerState);

	if (PlayerSlider == nullptr)
	{
		return;
	}
	PlayerSlider->OnValueChanged.AddDynamic(this, &UACPlayerVoiceControlEntry::OnSliderValueChanged);
	PlayerSlider->OnMouseCaptureEnd.AddDynamic(this, &UACPlayerVoiceControlEntry::OnSliderCaptureEnd);
}

void UACPlayerVoiceControlEntry::UpdatePlayer(AACPlayerState* InPlayerState)
{
	if (InPlayerState == nullptr)
	{
		UE_LOG(LogSY, Warning, TEXT("InPlayerState is nullptr2"));
		return;
	}
	if (PlayerNameText != nullptr)
	{
		PlayerNameText->SetText(FText::FromString(InPlayerState->GetPlayerName()));
	}
	else
	{
		UE_LOG(LogSY, Warning, TEXT("NameText is nullptr"));
	}


	if (PlayerAvatarImage != nullptr)
	{
		// Steam 아바타 가져오기
		FBPUniqueNetId UniqueNetId;
		UniqueNetId.SetUniqueNetId(InPlayerState->GetUniqueId().GetUniqueNetId());
		EBlueprintAsyncResultSwitch Result;
		UTexture2D* AvatarTexture = UAdvancedSteamFriendsLibrary::GetSteamFriendAvatar(
			UniqueNetId,
			Result,
			SteamAvatarSize::SteamAvatar_Medium
		);

		if (Result == EBlueprintAsyncResultSwitch::OnSuccess && AvatarTexture != nullptr)
		{
			PlayerAvatarImage->SetBrushFromTexture(AvatarTexture);
		}
		else
		{
			UE_LOG(LogSY, Warning, TEXT("Failed to get avatar for player"));
		}
	}
	else
	{
		UE_LOG(LogSY, Warning, TEXT("AvatarImage is nullptr"));
	}

	if (PlayerSlider != nullptr)
	{
		UACAdvancedFriendsGameInstance* GI = GetWorld()->GetGameInstance<UACAdvancedFriendsGameInstance>();
		if (GI == nullptr)
		{
			UE_LOG(LogSY, Warning, TEXT("GameInstance is nullptr"));
			return;
		}

		PlayerSlider->SetValue(GI->GetPlayerMicVolume(InPlayerState->GetPlayerName()));
	}
	else
	{
		UE_LOG(LogSY, Warning, TEXT("PlayerSlider is nullptr"));
	}

}

void UACPlayerVoiceControlEntry::OnSliderValueChanged(float Value)
{
	AACCharacter* Character = Cast<AACCharacter>(ItemData->PlayerState->GetPawn());
	if(Character == nullptr)
	{
		UE_LOG(LogSY, Warning, TEXT("Character is nullptr"));
		return;
	}

	Character->VOIPTalker->SetVOIPVolume(Value);
}

void UACPlayerVoiceControlEntry::OnSliderCaptureEnd()
{
	UACAdvancedFriendsGameInstance* GI = GetWorld()->GetGameInstance<UACAdvancedFriendsGameInstance>();
	if (GI == nullptr)
	{
		UE_LOG(LogSY, Warning, TEXT("GameInstance is nullptr"));
		return;
	}

	if (ItemData->PlayerState == nullptr)
	{
		UE_LOG(LogSY, Warning, TEXT("PlayerState is nullptr"));
		return;
	}

	if (PlayerSlider == nullptr)
	{
		UE_LOG(LogSY, Warning, TEXT("PlayerSlider is nullptr"));
		return;
	}

	// 슬라이더 조작이 끝나면 게임 인스턴스에 최종 볼륨 저장
	GI->SetPlayerMicVolume(ItemData->PlayerState->GetPlayerName(), PlayerSlider->GetValue());
}