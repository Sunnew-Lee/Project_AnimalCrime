

#include "UI/Score/ACScoreSlider.h"

#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Input/SSlider.h"

void UACScoreSlider::UpdateScore(float CurrentScore, float MaxScore)
{
	CurrentScore = FMath::Clamp(CurrentScore, 0.f, MaxScore);
	const float Percent = FMath::Clamp(CurrentScore / MaxScore, 0.f, 1.f);

	UE_LOG(LogTemp, Log, TEXT("[UACScoreWidget::UpdateScore]"));
	if (ScoreSlider == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("ScoreProgressBar nullptr"));
		return;
	}

	TSharedPtr<SSlider> SlateSlider = StaticCastSharedPtr<SSlider>(ScoreSlider->GetCachedWidget());

	EScoreState NewState;

	if (Percent <= 0.3f)
	{
		NewState = EScoreState::Sad;
	}
	else if (Percent <= 0.7f)
	{
		NewState = EScoreState::Normal;
	}
	else
	{
		NewState = EScoreState::Happy;
	}

	// 상태가 바뀔 때만 UI 변경
	if (NewState != CurrentState)
	{
		CurrentState = NewState;

		FSlateBrush Brush;

		switch (CurrentState)
		{
		case EScoreState::Sad:
			Brush.SetResourceObject(CryingImage);
			ScoreSlider->SetSliderBarColor(FLinearColor(1.0f, 0.6f, 0.6f));
			break;

		case EScoreState::Normal:
			Brush.SetResourceObject(NormalImage);
			ScoreSlider->SetSliderBarColor(FLinearColor(1.0f, 1.0f, 0.4f));
			break;

		case EScoreState::Happy:
			Brush.SetResourceObject(SmileImage);
			ScoreSlider->SetSliderBarColor(FLinearColor(0.2f, 1.0f, 0.2f));
			break;
		}

		Brush.ImageSize = HandleImageSize;
		SliderStyle.SetNormalThumbImage(Brush);
		SliderStyle.SetBarThickness(15.f);

		if (SlateSlider.IsValid())
		{
			SlateSlider->SetStyle(&SliderStyle);
		}
	}

	ScoreSlider->SetValue(CurrentScore);
	
	if (ScoreText == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("ScoreText nullptr"));
		return;
	}
	ScoreText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentScore, MaxScore)));
}
