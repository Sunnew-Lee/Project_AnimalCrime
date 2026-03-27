// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BoundItem/ACBoundItemWidget.h"
#include "Components/Image.h"

void UACBoundItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기에는 투명 처리
	ClearBoundItem();
}

void UACBoundItemWidget::SetBoundItem(UTexture2D* InItemImage)
{
	if (ItemImage == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("SetBoundItem: ItemImage is null"));
		return;
	}

	if (InItemImage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetBoundItem: InItemImage is null"));
		return;
	}

	// 이미지 설정
	ItemImage->SetBrushFromTexture(InItemImage);

	// 불투명하게 표시
	//ItemImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	//ItemImage->SetVisibility(ESlateVisibility::Visible);
	// 위젯 전체를 보이게
	SetRenderOpacity(1.0f);

	bHasItem = true;
}

void UACBoundItemWidget::ClearBoundItem()
{
	if (ItemImage == nullptr)
	{
		return;
	}

	// 투명하게 처리 (안보이게)
	//ItemImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
	//ItemImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	// 위젯 전체를 안보이게
	SetRenderOpacity(0.0f);

	bHasItem = false;
}
