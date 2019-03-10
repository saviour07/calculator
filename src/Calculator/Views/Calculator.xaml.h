// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "Views/Calculator.g.h"
#include "Converters/BooleanNegationConverter.h"
#include "Converters/ExpressionItemContainerStyle.h"
#include "Converters/ExpressionItemTemplateSelector.h"
#include "Converters/VisibilityNegationConverter.h"
#include "CalcViewModel/Common/Automation/NarratorNotifier.h"
#include "Controls/CalculatorButton.h"
#include "Controls/CalculationResult.h"
#include "Controls/OverflowTextBlock.h"
#include "CalcViewModel/HistoryViewModel.h"
#include "Views/CalculatorProgrammerDisplayPanel.xaml.h"
#include "Views/CalculatorProgrammerOperators.xaml.h"
#include "Views/CalculatorScientificAngleButtons.xaml.h"
#include "Views/HistoryList.xaml.h"
#include "Views/Memory.xaml.h"
#include "Views/OperatorsPanel.xaml.h"

namespace CalculatorApp
{
    struct FontTable
    {
        std::wstring numericSystem;
        double fullFont;
        double fullFontMin;
        double portraitMin;
        double snapFont;
        double fullNumPadFont;
        double snapScientificNumPadFont;
        double portraitScientificNumPadFont;
    };

    public delegate void FullscreenFlyoutClosedEventHandler();

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class Calculator sealed
    {
    public:
        event FullscreenFlyoutClosedEventHandler^ FullscreenFlyoutClosed;

        Calculator();
        property CalculatorApp::ViewModel::StandardCalculatorViewModel^ Model
        {
            CalculatorApp::ViewModel::StandardCalculatorViewModel^ get() {
                return static_cast<CalculatorApp::ViewModel::StandardCalculatorViewModel^>(this->DataContext);
            }
        }

        DEPENDENCY_PROPERTY_OWNER(Calculator);
        DEPENDENCY_PROPERTY_WITH_DEFAULT_AND_CALLBACK(bool, IsStandard, false);
        DEPENDENCY_PROPERTY_WITH_DEFAULT_AND_CALLBACK(bool, IsScientific, false);
        DEPENDENCY_PROPERTY_WITH_DEFAULT_AND_CALLBACK(bool, IsProgrammer, false);

        COMMAND_FOR_METHOD(HistoryButtonPressed, Calculator::ToggleHistoryFlyout);

        void AnimateCalculator(bool resultAnimate);
        void InitializeHistoryView(CalculatorApp::ViewModel::HistoryViewModel^ historyVM);
        void UpdatePanelViewState();
        void UnregisterEventHandlers();

        void CloseHistoryFlyout();
        void CloseMemoryFlyout();

        void SetDefaultFocus();

    private:
        void OnLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        void LoadResourceStrings();

        void UpdateViewState();
        void UpdateMemoryState();
        void UpdateHistoryState();

        void CalculationResultsOnSelected(_In_ Platform::Object^ sender);
        void OnContextRequested(Windows::UI::Xaml::UIElement^ sender, Windows::UI::Xaml::Input::ContextRequestedEventArgs^ e);
        void OnContextCanceled(Windows::UI::Xaml::UIElement^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void OnIsScientificPropertyChanged(bool oldValue, bool newValue);
        void OnIsProgrammerPropertyChanged(bool oldValue, bool newValue);
        void OnIsStandardPropertyChanged(bool oldValue, bool newValue);
        void OnIsInErrorPropertyChanged();
        void OnCalcPropertyChanged(_In_ Platform::Object^ sender, _In_ Windows::UI::Xaml::Data::PropertyChangedEventArgs^ e);
        void OnStoryboardCompleted(_In_ Platform::Object^ sender, _In_ Platform::Object^ e);
        void OnLayoutStateChanged(_In_ Platform::Object^ sender, _In_ Platform::Object^ e);
        void EnsureScientific();
        void EnsureProgrammer();
        void SetFontSizeResources();
        std::wstring GetCurrentLayoutState();

    private:
        Windows::UI::Xaml::Controls::ListView^ m_tokenList;
        Windows::UI::Xaml::Controls::MenuFlyout^ m_displayFlyout;
        bool m_doAnimate;
        bool m_resultAnimate;
        bool m_isLastAnimatedInScientific;
        bool m_isLastAnimatedInProgrammer;
        bool m_IsLastFlyoutMemory = false;
        bool m_IsLastFlyoutHistory = false;

        Platform::String^ m_openMemoryFlyoutAutomationName;
        Platform::String^ m_closeMemoryFlyoutAutomationName;
        Platform::String^ m_openHistoryFlyoutAutomationName;
        Platform::String^ m_closeHistoryFlyoutAutomationName;

        Windows::UI::Xaml::Controls::PivotItem^ m_pivotItem;
        bool m_IsDigit = false;
        Memory^ m_memory;
        void HistoryFlyout_Opened(_In_ Platform::Object ^sender, _In_ Platform::Object ^args);
        void HistoryFlyout_Closing(_In_ Windows::UI::Xaml::Controls::Primitives::FlyoutBase^ sender, _In_ Windows::UI::Xaml::Controls::Primitives::FlyoutBaseClosingEventArgs^ args);
        void HistoryFlyout_Closed(_In_ Platform::Object ^sender, _In_ Platform::Object ^args);
        void OnHideHistoryClicked();
        void OnHideMemoryClicked();
        void OnHistoryItemClicked(_In_ CalculatorApp::ViewModel::HistoryItemViewModel^ e);
        void ToggleHistoryFlyout(Platform::Object^ parameter);
        void ToggleMemoryFlyout();
        CalculatorApp::HistoryList^ m_historyList;
        bool m_fIsHistoryFlyoutOpen;
        bool m_fIsMemoryFlyoutOpen;
        void OnMemoryFlyoutOpened(_In_ Platform::Object ^sender, _In_ Platform::Object ^args);
        void OnMemoryFlyoutClosing(_In_ Windows::UI::Xaml::Controls::Primitives::FlyoutBase^ sender, _In_ Windows::UI::Xaml::Controls::Primitives::FlyoutBaseClosingEventArgs^ args);
        void OnMemoryFlyoutClosed(_In_ Platform::Object ^sender, _In_ Platform::Object ^args);
        void SetChildAsMemory();
        void SetChildAsHistory();
        Memory^ GetMemory();
        void EnableControls(bool enable);
        void EnableMemoryControls(bool enable);
        void OnMemoryFlyOutTapped(_In_ Platform::Object^ sender, _In_ Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
        void OnHistoryFlyOutTapped(_In_ Platform::Object^ sender, _In_ Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
        void expressionContainer_LayoutUpdated(_In_ Platform::Object^ sender, _In_ Platform::Object^ e);
        bool IsValidRegularExpression(std::wstring str);
        void DockPanelTapped(_In_ Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
        void OnResultsLayoutChanged(_In_ Platform::Object^ sender, _In_ Platform::Object^ e);
        void SetResultStyles();
        void OnErrorLayoutCompleted(_In_ Platform::Object^ sender, _In_ Platform::Object^ e);
        void OnHistoryAccessKeyInvoked(_In_ Windows::UI::Xaml::UIElement^ sender, _In_ Windows::UI::Xaml::Input::AccessKeyInvokedEventArgs^ args);
        void OnMemoryAccessKeyInvoked(_In_ Windows::UI::Xaml::UIElement^ sender, _In_ Windows::UI::Xaml::Input::AccessKeyInvokedEventArgs^ args);
        void DockPivot_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);

    private:
        static constexpr auto m_displayContextMenuStr = L"DisplayContextMenu";
        static constexpr auto m_copyMenuItemStr = L"copyMenuItem";
        static constexpr auto m_pasteMenuItemStr = L"pasteMenuItem";
        static constexpr auto m_memoryButtonOpenStr = L"MemoryButton_Open";
        static constexpr auto m_memoryButtonCloseStr = L"MemoryButton_Close";
        static constexpr auto m_historyButtonOpenStr = L"HistoryButton_Open";
        static constexpr auto m_historyButtonCloseStr = L"HistoryButton_Close";

        static constexpr auto m_arabStr = L"Arab";
        static constexpr auto m_arabExtStr = L"ArabExt";
        static constexpr auto m_bengStr = L"Beng";
        static constexpr auto m_devaStr = L"Deva";
        static constexpr auto m_gujrStr = L"Gujr";
        static constexpr auto m_khmrStr = L"Khmr";
        static constexpr auto m_kndaStr = L"Knda";
        static constexpr auto m_laooStr = L"Laoo";
        static constexpr auto m_latnStr = L"Latn";
        static constexpr auto m_mlymStr = L"Mlym";
        static constexpr auto m_mymrStr = L"Mymr";
        static constexpr auto m_oryaStr = L"Orya";
        static constexpr auto m_tamlDecStr = L"TamlDec";
        static constexpr auto m_teluStr = L"Telu";
        static constexpr auto m_thaiStr = L"Thai";
        static constexpr auto m_tibtStr = L"Tibt";
        static constexpr auto m_defaultStr = L"Default";

        static constexpr auto m_resultFullFontSizeStr = L"ResultFullFontSize";
        static constexpr auto m_resultFullMinFontSizeStr = L"ResultFullMinFontSize";
        static constexpr auto m_resultPortraitMinFontSizeStr = L"ResultPortraitMinFontSize";
        static constexpr auto m_resultSnapFontSizeStr = L"ResultSnapFontSize";
        static constexpr auto m_calcButtonCaptionSizeOverrideStr = L"CalcButtonCaptionSizeOverride";
        static constexpr auto m_calcButtonScientificSnapCaptionSizeOverrideStr = L"CalcButtonScientificSnapCaptionSizeOverride";
        static constexpr auto m_calcButtonScientificPortraitCaptionSizeOverrideStr = L"CalcButtonScientificPortraitCaptionSizeOverride";

        static constexpr auto m_historyPaneStr = L"HistoryPane";
        static constexpr auto m_memoryPaneStr = L"MemoryPane";

        static constexpr auto m_windowsUIXamlControlsPrimitivesFlyoutBaseStr = L"Windows.UI.Xaml.Controls.Primitives.FlyoutBase";
        static constexpr auto m_closingStr = L"Closing";

        static constexpr auto m_programmerStr = L"Programmer";
        static constexpr auto m_scientificStr = L"Scientific";
        static constexpr auto m_standardStr = L"Standard";

        static constexpr auto m_errorLayoutStr = L"ErrorLayout";
        static constexpr auto m_noErrorLayoutStr = L"NoErrorLayout";

        static constexpr auto m_scientificAngleButtonsStr = L"ScientificAngleButtons";
        static constexpr auto m_programmerOperatorsStr = L"ProgrammerOperators";
        static constexpr auto m_programmerDisplayPanelStr = L"ProgrammerDisplayPanel";

        static constexpr auto m_validRegexPatternZeroStr = L"[-]{0,1}[0-9]{0,}[.]{0,1}[0-9]{0,}";
        static constexpr auto m_validRegexPatternOneStr = L"[-]{0,1}[0-9]{0,}[.]{0,1}[0-9]{0,}[e]{1}[+]{1}[0-9]{1,}";
        static constexpr auto m_validRegexPatternTwoStr = L"[-]{0,1}[0-9]{0,}[.]{0,1}[0-9]{0,}[e]{1}[-]{1}[0-9]{1,}";
};
}
