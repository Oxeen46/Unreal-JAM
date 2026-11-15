#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionsMenuWidget.generated.h"

class UButton;
class UCheckBox;
class UComboBoxString;
class USlider;
class UTextBlock;
class UWidgetSwitcher;
class UGameSettingsSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOptionsBack);

UCLASS()
class EPICAIMEGAJAM_API UOptionsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Bind this in your Blueprint if you want to do something when Back is pressed.
	// If nothing is bound, the widget just removes itself.
	UPROPERTY(BlueprintAssignable, Category="Options")
	FOnOptionsBack OnBack;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// Every widget below is optional. Add the ones you want in the Designer (Is Variable ticked).

	// Tabs
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UWidgetSwitcher> CategorySwitcher;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> VideoTabButton;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> AudioTabButton;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> ControlsTabButton;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> AccessibilityTabButton;

	// Video
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UComboBoxString> WindowModeCombo;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UComboBoxString> ResolutionCombo;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UCheckBox> VSyncCheck;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UComboBoxString> FrameRateCombo;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UComboBoxString> QualityCombo;

	// Audio (sliders run 0 to 1)
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<USlider> MasterSlider;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<USlider> MusicSlider;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<USlider> SfxSlider;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> MasterValueText;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> MusicValueText;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> SfxValueText;

	// Controls
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<USlider> MouseSensitivitySlider;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> MouseSensitivityText;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UCheckBox> InvertYCheck;

	// Accessibility
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<USlider> RealityEffectSlider;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> RealityEffectText;

	// Buttons and feedback
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> ApplyButton;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> DefaultsButton;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> BackButton;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> StatusText;

private:
	UFUNCTION() void HandleVideoTab();
	UFUNCTION() void HandleAudioTab();
	UFUNCTION() void HandleControlsTab();
	UFUNCTION() void HandleAccessibilityTab();
	UFUNCTION() void HandleApply();
	UFUNCTION() void HandleDefaults();
	UFUNCTION() void HandleBack();

	UFUNCTION() void HandleMasterChanged(float Value);
	UFUNCTION() void HandleMusicChanged(float Value);
	UFUNCTION() void HandleSfxChanged(float Value);
	UFUNCTION() void HandleMouseChanged(float Value);
	UFUNCTION() void HandleRealityChanged(float Value);

	void PopulateCombos();
	void LoadValuesIntoControls();
	void SetControlsToDefaults();
	void SetTab(int32 Index);
	void SetStatus(const FString& Text);
	UGameSettingsSubsystem* GetSettings() const;
};