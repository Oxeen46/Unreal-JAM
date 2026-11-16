#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

class UImage;

// A small dot in the middle of the screen (built in C++, no Blueprint needed).
// Idle: small, soft white. When the interaction ray hits something usable: grows a little and turns warm gold.
UCLASS()
class EPICAIMEGAJAM_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetFocused(bool bInFocused) { bFocused = bInFocused; }

	// Dot diameter in pixels (idle / looking at something usable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair")
	float IdleSize = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair")
	float FocusedSize = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair")
	FLinearColor IdleColor = FLinearColor(1.f, 1.f, 1.f, 0.55f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair")
	FLinearColor FocusedColor = FLinearColor(1.f, 0.78f, 0.35f, 0.95f);

protected:
	virtual bool Initialize() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY()
	TObjectPtr<UImage> Dot;

	bool bFocused = false;
	float CurrentSize = 4.f;
};
