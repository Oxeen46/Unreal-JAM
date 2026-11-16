#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndScreenWidget.generated.h"

class UTextBlock;

// Full-screen black card with a centered message that fades in (built in C++, no Blueprint).
UCLASS()
class EPICAIMEGAJAM_API UEndScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Show(const FText& Message, float FadeTime);

protected:
	virtual bool Initialize() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> MessageText;

	float Elapsed = 0.f;
	float Fade = 2.f;
};
