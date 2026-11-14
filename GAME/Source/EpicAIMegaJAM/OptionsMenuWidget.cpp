#include "OptionsMenuWidget.h"
#include "GameSettingsSubsystem.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"

namespace
{
	const TCHAR* WindowModeNames[] = { TEXT("Fullscreen"), TEXT("Windowed Fullscreen"), TEXT("Windowed") };
	const EWindowMode::Type WindowModeValues[] = { EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen, EWindowMode::Windowed };
	const TCHAR* QualityNames[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic"), TEXT("Cinematic") };

	struct FRateOption { const TCHAR* Label; float Value; };
	const FRateOption RateOptions[] = {
		{ TEXT("30"), 30.f }, { TEXT("60"), 60.f }, { TEXT("90"), 90.f },
		{ TEXT("120"), 120.f }, { TEXT("144"), 144.f }, { TEXT("Unlimited"), 0.f }
	};

	// The mouse slider runs 0 to 1 and maps to this range
	constexpr float MinSensitivity = 0.2f;
	constexpr float MaxSensitivity = 3.f;

	float SliderToSensitivity(float Slider) { return FMath::Lerp(MinSensitivity, MaxSensitivity, Slider); }
	float SensitivityToSlider(float Sensitivity) { return (Sensitivity - MinSensitivity) / (MaxSensitivity - MinSensitivity); }

	FString ResolutionText(const FIntPoint& Resolution)
	{
		return FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y);
	}

	bool ParseResolution(const FString& Text, FIntPoint& Out)
	{
		FString Width, Height;
		if (!Text.Split(TEXT(" x "), &Width, &Height)) return false;
		Out = FIntPoint(FCString::Atoi(*Width), FCString::Atoi(*Height));
		return Out.X > 0 && Out.Y > 0;
	}

	FString PercentText(float Value)
	{
		return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.f));
	}
}

UGameSettingsSubsystem* UOptionsMenuWidget::GetSettings() const
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UGameSettingsSubsystem>() : nullptr;
}

void UOptionsMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);

	if (VideoTabButton) VideoTabButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::HandleVideoTab);
	if (AudioTabButton) AudioTabButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::HandleAudioTab);
	if (ControlsTabButton) ControlsTabButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::HandleControlsTab);
	if (AccessibilityTabButton) AccessibilityTabButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::HandleAccessibilityTab);

	if (ApplyButton) ApplyButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::HandleApply);
	if (DefaultsButton) DefaultsButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::HandleDefaults);
	if (BackButton) BackButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::HandleBack);

	if (MasterSlider) MasterSlider->OnValueChanged.AddDynamic(this, &UOptionsMenuWidget::HandleMasterChanged);
	if (MusicSlider) MusicSlider->OnValueChanged.AddDynamic(this, &UOptionsMenuWidget::HandleMusicChanged);
	if (SfxSlider) SfxSlider->OnValueChanged.AddDynamic(this, &UOptionsMenuWidget::HandleSfxChanged);
	if (MouseSensitivitySlider) MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UOptionsMenuWidget::HandleMouseChanged);
	if (RealityEffectSlider) RealityEffectSlider->OnValueChanged.AddDynamic(this, &UOptionsMenuWidget::HandleRealityChanged);

	PopulateCombos();
}

void UOptionsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	LoadValuesIntoControls();
	SetTab(0);
	SetStatus(FString());
}

FReply UOptionsMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Right)
	{
		HandleBack();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UOptionsMenuWidget::PopulateCombos()
{
	if (WindowModeCombo)
	{
		WindowModeCombo->ClearOptions();
		for (const TCHAR* Name : WindowModeNames) WindowModeCombo->AddOption(Name);
	}

	if (ResolutionCombo)
	{
		TArray<FIntPoint> Resolutions;
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

		TArray<FIntPoint> Windowed;
		UKismetSystemLibrary::GetConvenientWindowedResolutions(Windowed);
		Resolutions.Append(Windowed);

		if (Resolutions.Num() == 0)
		{
			Resolutions = { FIntPoint(1280, 720), FIntPoint(1600, 900), FIntPoint(1920, 1080), FIntPoint(2560, 1440), FIntPoint(3840, 2160) };
		}

		Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B) { return A.X != B.X ? A.X < B.X : A.Y < B.Y; });

		ResolutionCombo->ClearOptions();
		for (int32 i = 0; i < Resolutions.Num(); ++i)
		{
			if (i > 0 && Resolutions[i] == Resolutions[i - 1]) continue;
			ResolutionCombo->AddOption(ResolutionText(Resolutions[i]));
		}
	}

	if (FrameRateCombo)
	{
		FrameRateCombo->ClearOptions();
		for (const FRateOption& Rate : RateOptions) FrameRateCombo->AddOption(Rate.Label);
	}

	if (QualityCombo)
	{
		QualityCombo->ClearOptions();
		for (const TCHAR* Name : QualityNames) QualityCombo->AddOption(Name);
	}
}

void UOptionsMenuWidget::LoadValuesIntoControls()
{
	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		if (WindowModeCombo)
		{
			for (int32 i = 0; i < UE_ARRAY_COUNT(WindowModeValues); ++i)
			{
				if (WindowModeValues[i] == GUS->GetFullscreenMode())
				{
					WindowModeCombo->SetSelectedOption(WindowModeNames[i]);
				}
			}
		}

		if (ResolutionCombo)
		{
			const FString Current = ResolutionText(GUS->GetScreenResolution());
			if (ResolutionCombo->FindOptionIndex(Current) == INDEX_NONE)
			{
				ResolutionCombo->AddOption(Current);
			}
			ResolutionCombo->SetSelectedOption(Current);
		}

		if (VSyncCheck) VSyncCheck->SetIsChecked(GUS->IsVSyncEnabled());

		if (FrameRateCombo)
		{
			const float Limit = GUS->GetFrameRateLimit();
			FString Label = TEXT("Unlimited");
			for (const FRateOption& Rate : RateOptions)
			{
				if (FMath::IsNearlyEqual(Rate.Value, Limit)) Label = Rate.Label;
			}
			FrameRateCombo->SetSelectedOption(Label);
		}

		if (QualityCombo)
		{
			const int32 Level = GUS->GetOverallScalabilityLevel();
			if (Level >= 0 && Level < UE_ARRAY_COUNT(QualityNames)) QualityCombo->SetSelectedIndex(Level);
			else QualityCombo->ClearSelection();
		}
	}

	if (const UGameSettingsSubsystem* Settings = GetSettings())
	{
		const FGameOptions Options = Settings->GetOptions();
		if (MasterSlider) MasterSlider->SetValue(Options.MasterVolume);
		if (MusicSlider) MusicSlider->SetValue(Options.MusicVolume);
		if (SfxSlider) SfxSlider->SetValue(Options.SfxVolume);
		if (MouseSensitivitySlider) MouseSensitivitySlider->SetValue(SensitivityToSlider(Options.MouseSensitivity));
		if (InvertYCheck) InvertYCheck->SetIsChecked(Options.bInvertY);
		if (RealityEffectSlider) RealityEffectSlider->SetValue(Options.RealityEffect);

		HandleMasterChanged(Options.MasterVolume);
		HandleMusicChanged(Options.MusicVolume);
		HandleSfxChanged(Options.SfxVolume);
		HandleMouseChanged(SensitivityToSlider(Options.MouseSensitivity));
		HandleRealityChanged(Options.RealityEffect);
	}
}

// Puts defaults on the controls only. Press Apply to keep them.
void UOptionsMenuWidget::SetControlsToDefaults()
{
	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		if (WindowModeCombo) WindowModeCombo->SetSelectedOption(WindowModeNames[1]);
		if (ResolutionCombo)
		{
			const FString Desktop = ResolutionText(GUS->GetDesktopResolution());
			if (ResolutionCombo->FindOptionIndex(Desktop) == INDEX_NONE) ResolutionCombo->AddOption(Desktop);
			ResolutionCombo->SetSelectedOption(Desktop);
		}
		if (VSyncCheck) VSyncCheck->SetIsChecked(true);
		if (FrameRateCombo) FrameRateCombo->SetSelectedOption(TEXT("Unlimited"));
		if (QualityCombo) QualityCombo->SetSelectedIndex(2);
	}

	const FGameOptions Defaults;
	if (MasterSlider) MasterSlider->SetValue(Defaults.MasterVolume);
	if (MusicSlider) MusicSlider->SetValue(Defaults.MusicVolume);
	if (SfxSlider) SfxSlider->SetValue(Defaults.SfxVolume);
	if (MouseSensitivitySlider) MouseSensitivitySlider->SetValue(SensitivityToSlider(Defaults.MouseSensitivity));
	if (InvertYCheck) InvertYCheck->SetIsChecked(Defaults.bInvertY);
	if (RealityEffectSlider) RealityEffectSlider->SetValue(Defaults.RealityEffect);

	HandleMasterChanged(Defaults.MasterVolume);
	HandleMusicChanged(Defaults.MusicVolume);
	HandleSfxChanged(Defaults.SfxVolume);
	HandleMouseChanged(SensitivityToSlider(Defaults.MouseSensitivity));
	HandleRealityChanged(Defaults.RealityEffect);
}

void UOptionsMenuWidget::HandleApply()
{
	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		if (WindowModeCombo)
		{
			for (int32 i = 0; i < UE_ARRAY_COUNT(WindowModeNames); ++i)
			{
				if (WindowModeCombo->GetSelectedOption() == WindowModeNames[i]) GUS->SetFullscreenMode(WindowModeValues[i]);
			}
		}

		FIntPoint Resolution;
		if (ResolutionCombo && ParseResolution(ResolutionCombo->GetSelectedOption(), Resolution))
		{
			GUS->SetScreenResolution(Resolution);
		}

		if (VSyncCheck) GUS->SetVSyncEnabled(VSyncCheck->IsChecked());

		if (FrameRateCombo)
		{
			for (const FRateOption& Rate : RateOptions)
			{
				if (FrameRateCombo->GetSelectedOption() == Rate.Label) GUS->SetFrameRateLimit(Rate.Value);
			}
		}

		if (QualityCombo && QualityCombo->GetSelectedIndex() >= 0)
		{
			GUS->SetOverallScalabilityLevel(QualityCombo->GetSelectedIndex());
		}

		GUS->ApplySettings(false);
		GUS->SaveSettings();
	}

	if (UGameSettingsSubsystem* Settings = GetSettings())
	{
		FGameOptions Options = Settings->GetOptions();
		if (MasterSlider) Options.MasterVolume = MasterSlider->GetValue();
		if (MusicSlider) Options.MusicVolume = MusicSlider->GetValue();
		if (SfxSlider) Options.SfxVolume = SfxSlider->GetValue();
		if (MouseSensitivitySlider) Options.MouseSensitivity = SliderToSensitivity(MouseSensitivitySlider->GetValue());
		if (InvertYCheck) Options.bInvertY = InvertYCheck->IsChecked();
		if (RealityEffectSlider) Options.RealityEffect = RealityEffectSlider->GetValue();
		Settings->SetOptions(Options);
	}

	SetStatus(TEXT("Settings saved."));
}

void UOptionsMenuWidget::HandleDefaults()
{
	SetControlsToDefaults();
	SetStatus(TEXT("Defaults loaded. Press Apply to keep them."));
}

void UOptionsMenuWidget::HandleBack()
{
	// Anything not applied is discarded
	if (OnBack.IsBound()) OnBack.Broadcast();
	else RemoveFromParent();
}

void UOptionsMenuWidget::HandleVideoTab()         { SetTab(0); }
void UOptionsMenuWidget::HandleAudioTab()         { SetTab(1); }
void UOptionsMenuWidget::HandleControlsTab()      { SetTab(2); }
void UOptionsMenuWidget::HandleAccessibilityTab() { SetTab(3); }

void UOptionsMenuWidget::SetTab(int32 Index)
{
	if (CategorySwitcher && Index < CategorySwitcher->GetNumWidgets())
	{
		CategorySwitcher->SetActiveWidgetIndex(Index);
	}
}

void UOptionsMenuWidget::SetStatus(const FString& Text)
{
	if (StatusText) StatusText->SetText(FText::FromString(Text));
}

void UOptionsMenuWidget::HandleMasterChanged(float Value)
{
	if (MasterValueText) MasterValueText->SetText(FText::FromString(PercentText(Value)));
}

void UOptionsMenuWidget::HandleMusicChanged(float Value)
{
	if (MusicValueText) MusicValueText->SetText(FText::FromString(PercentText(Value)));
}

void UOptionsMenuWidget::HandleSfxChanged(float Value)
{
	if (SfxValueText) SfxValueText->SetText(FText::FromString(PercentText(Value)));
}

void UOptionsMenuWidget::HandleMouseChanged(float Value)
{
	if (MouseSensitivityText) MouseSensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.1fx"), SliderToSensitivity(Value))));
}

void UOptionsMenuWidget::HandleRealityChanged(float Value)
{
	if (RealityEffectText) RealityEffectText->SetText(FText::FromString(PercentText(Value)));
}