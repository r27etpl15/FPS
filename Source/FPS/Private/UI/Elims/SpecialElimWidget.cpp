// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Elims/SpecialElimWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void USpecialElimWidget::InitializeWidget(const FString& ElimMessage, UTexture2D* ElimTexture)
{
	if (IsValid(Text_ElimMessage))
	{
		Text_ElimMessage->SetText(FText::FromString(ElimMessage));
	}
	if (IsValid(Image_ElimType))
	{
		Image_ElimType->SetBrushFromTexture(ElimTexture);
	}
}

void USpecialElimWidget::CenterWidget(UUserWidget* Widget, float VerticalRatio)
{
	if (!IsValid(Widget)) { return; }
	
	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(Widget);
	const float VerticalFraction = VerticalRatio == 0.0f ? 1.0f : VerticalRatio * 2.0f;	// 垂直占比 [0 ~ 1]
	FVector2D CenterPosition(ViewportSize.X / 2.0f, VerticalFraction * ViewportSize.Y / 2.0f);
	
	Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));	// 将控件中心对齐到视口中心
	Widget->SetPositionInViewport(CenterPosition, true);
}
