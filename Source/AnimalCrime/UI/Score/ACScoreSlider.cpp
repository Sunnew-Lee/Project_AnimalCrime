

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
	if (bIsHappy == true && Percent <= 0.3)
	{
		bIsHappy = false;
		FSlateBrush Brush;
		Brush.SetResourceObject(CryingImage);
		Brush.ImageSize = HandleImageSize;
		SliderStyle.SetNormalThumbImage(Brush);
		SliderStyle.SetBarThickness(15.f);
		ScoreSlider->SetSliderBarColor(FLinearColor(1.0f, 0.6f, 0.6f));
		if (SlateSlider.IsValid() == true)
		{
			SlateSlider->SetStyle(&SliderStyle);
		}
	}
	else if (bIsHappy == false && Percent > 0.3)
	{
		bIsHappy = true;
		FSlateBrush Brush;
		Brush.SetResourceObject(SmileImage);
		Brush.ImageSize = HandleImageSize;
		SliderStyle.SetNormalThumbImage(Brush);
		SliderStyle.SetBarThickness(15.f);
		ScoreSlider->SetSliderBarColor(FLinearColor(0.2f, 1.0f, 0.2f));
		if (SlateSlider.IsValid() == true)
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
