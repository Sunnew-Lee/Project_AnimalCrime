
#include "UI/Spectator/ACSpectatorScreen.h"
#include "UI/Score/ACScoreSlider.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "AdvancedSteamFriendsLibrary.h"
#include "AdvancedFriendsLibrary.h"
#include "Game/ACMainGameState.h"
#include "Game/ACPlayerState.h"

void UACSpectatorScreen::BindGameState()
{
	if (UWorld* World = GetWorld())
	{
		if (AACMainGameState* GS = World->GetGameState<AACMainGameState>())
		{
			GS->OnScoreChanged.AddDynamic(this, &UACSpectatorScreen::HandleScoreChanged);
			GS->OnEscapeCountChanged.AddDynamic(this, &UACSpectatorScreen::HandleEscapeCountChanged);

			HandleScoreChanged(GS->GetTeamScore());
			HandleEscapeCountChanged(GS->GetEscapedCount());
		}
	}
}


void UACSpectatorScreen::HandleScoreChanged(float NewScore)
{
	if (ScoreSlider == nullptr)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (AACMainGameState* MainGameState = World->GetGameState<AACMainGameState>())
		{
			//WBP_Score->UpdateScore(NewScore, MainGameState->GetMaxScore());

			ScoreSlider->UpdateScore(NewScore, MainGameState->GetMaxScore());
		}
	}
}

void UACSpectatorScreen::HandleEscapeCountChanged(int32 NewCount)
{
	if (EscapeNumText == nullptr)
	{
		return;
	}
	FString EscapeText = FString::Printf(TEXT("탈출 인원: %d명"), NewCount);
	EscapeNumText->SetText(FText::FromString(EscapeText));
}

void UACSpectatorScreen::SetFriend(AACPlayerState* PS)
{
	if (FriendNameText == nullptr)
	{
		return;
	}

	if (FriendImage == nullptr)
	{
		return;
	}

	//플레이어 이름 12자까지만 표기
	FString PlayerName = PS->GetPlayerName();

	if (PlayerName.Len() > 12)
	{
		PlayerName = PlayerName.Left(12) + TEXT("...");
	}

	FriendNameText->SetText(FText::FromString(PlayerName));

	// Steam 아바타 가져오기
	FBPUniqueNetId UniqueNetId;
	UniqueNetId.SetUniqueNetId(PS->GetUniqueId().GetUniqueNetId());

	EBlueprintAsyncResultSwitch Result;
	UTexture2D* AvatarTexture = UAdvancedSteamFriendsLibrary::GetSteamFriendAvatar(
		UniqueNetId,
		Result,
		SteamAvatarSize::SteamAvatar_Medium
	);

	if (Result != EBlueprintAsyncResultSwitch::OnSuccess || AvatarTexture == nullptr)
	{
		return;
	}

	FriendImage->SetBrushFromTexture(AvatarTexture);
}
