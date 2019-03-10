// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include "Calculator.xaml.h"
#include "CalculatorStandardOperators.xaml.h"
#include "CalcViewModel/Common/TraceLogger.h"
#include "CalcViewModel/Common/CopyPasteManager.h"
#include "CalcViewModel/StandardCalculatorViewModel.h"
#include "CalcViewModel/ViewState.h"
#include "CalcViewModel/Common/LocalizationSettings.h"
#include "Memory.xaml.h"
#include "HistoryList.xaml.h"

using namespace CalculatorApp;
using namespace CalculatorApp::Common;
using namespace CalculatorApp::Converters;
using namespace CalculatorApp::Controls;
using namespace CalculatorApp::ViewModel;

using namespace Concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Globalization::NumberFormatting;
using namespace Windows::System;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Automation;
using namespace Windows::UI::Xaml::Automation::Peers;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::System::Threading;
using namespace Windows::UI::ViewManagement;

DEPENDENCY_PROPERTY_INITIALIZATION(Calculator, IsStandard);
DEPENDENCY_PROPERTY_INITIALIZATION(Calculator, IsScientific);
DEPENDENCY_PROPERTY_INITIALIZATION(Calculator, IsProgrammer);

Calculator::Calculator() :
m_doAnimate(false),
m_isLastAnimatedInScientific(false),
m_isLastAnimatedInProgrammer(false),
m_resultAnimate(false)
{
    SetFontSizeResources();
    InitializeComponent();
    LoadResourceStrings();

    if (LocalizationService::GetInstance()->IsRtlLayout())
    {
        HistoryButton->HorizontalAlignment = ::HorizontalAlignment::Left;
    }

    m_displayFlyout = static_cast<MenuFlyout^>(Resources->Lookup(ref new String(m_displayContextMenuStr)));
    auto resLoader = AppResourceProvider::GetInstance();
    CopyMenuItem->Text = resLoader.GetResourceString(ref new String(m_copyMenuItemStr));
    PasteMenuItem->Text = resLoader.GetResourceString(ref new String(m_pasteMenuItemStr));
}

void Calculator::LoadResourceStrings()
{
    auto resProvider = AppResourceProvider::GetInstance();
    m_openMemoryFlyoutAutomationName = resProvider.GetResourceString(ref new String(m_memoryButtonOpenStr));
    m_closeMemoryFlyoutAutomationName = resProvider.GetResourceString(ref new String(m_memoryButtonCloseStr));
    m_openHistoryFlyoutAutomationName = resProvider.GetResourceString(ref new String(m_historyButtonOpenStr));
    m_closeHistoryFlyoutAutomationName = resProvider.GetResourceString(ref new String(m_historyButtonCloseStr));
    AutomationProperties::SetName(MemoryButton, m_openMemoryFlyoutAutomationName);
    AutomationProperties::SetName(HistoryButton, m_openHistoryFlyoutAutomationName);
}

void Calculator::InitializeHistoryView(_In_ HistoryViewModel^ historyVM)
{
    if (m_historyList == nullptr)
    {
        m_historyList = ref new HistoryList();
        m_historyList->DataContext = historyVM;
        historyVM->HideHistoryClicked += ref new ViewModel::HideHistoryClickedHandler(this, &Calculator::OnHideHistoryClicked);
        historyVM->HistoryItemClicked += ref new ViewModel::HistoryItemClickedHandler(this, &Calculator::OnHistoryItemClicked);
    }
}

void Calculator::SetFontSizeResources()
{
    // Since we need different font sizes for different numeric system,
    // we use a table of optimal font sizes for each numeric system.
    static const FontTable fontTables[] = {
            { m_arabStr, 104, 29.333, 23, 40, 56, 40, 56 },
            { m_arabExtStr, 104, 29.333, 23, 40, 56, 40, 56 },
            { m_bengStr, 104, 26, 17, 40, 56, 40, 56 },
            { m_devaStr, 104, 29.333, 20.5, 40, 56, 40, 56 },
            { m_gujrStr, 104, 29.333, 18.5, 40, 56, 40, 56 },
            { m_khmrStr, 104, 29.333, 19.5, 40, 56, 40, 56 },
            { m_kndaStr, 104, 25, 17, 40, 56, 40, 56 },
            { m_laooStr, 104, 28, 18, 40, 56, 40, 56 },
            { m_latnStr, 104, 29.333, 23, 40, 56, 40, 56 },
            { m_mlymStr, 80, 22, 15.5, 30, 56, 35, 48 },
            { m_mymrStr, 104, 29.333, 20, 35, 48, 36, 48 },
            { m_oryaStr, 88, 26, 20, 40, 56, 40, 56 },
            { m_tamlDecStr, 77, 25, 16, 28, 48, 34, 48 },
            { m_teluStr, 104, 25, 16.5, 40, 56, 40, 56 },
            { m_thaiStr, 104, 28, 18, 40, 56, 40, 56 },
            { m_tibtStr, 104, 29.333, 20, 40, 56, 40, 56 },
            { m_defaultStr, 104, 29.333, 23, 40, 56, 40, 56 }
    };

    DecimalFormatter^ formatter = LocalizationService::GetRegionalSettingsAwareDecimalFormatter();

    const FontTable* currentItem = fontTables;
    while (currentItem->numericSystem.compare(m_defaultStr) != 0 &&
        currentItem->numericSystem.compare(formatter->NumeralSystem->Data()) != 0)
    {
        currentItem++;
    }

    this->Resources->Insert(StringReference(m_resultFullFontSizeStr), currentItem->fullFont);
    this->Resources->Insert(StringReference(m_resultFullMinFontSizeStr), currentItem->fullFontMin);
    this->Resources->Insert(StringReference(m_resultPortraitMinFontSizeStr), currentItem->portraitMin);
    this->Resources->Insert(StringReference(m_resultSnapFontSizeStr), currentItem->snapFont);
    this->Resources->Insert(StringReference(m_calcButtonCaptionSizeOverrideStr), currentItem->fullNumPadFont);
    this->Resources->Insert(StringReference(m_calcButtonScientificSnapCaptionSizeOverrideStr), currentItem->snapScientificNumPadFont);
    this->Resources->Insert(StringReference(m_calcButtonScientificPortraitCaptionSizeOverrideStr), currentItem->portraitScientificNumPadFont);
}

void Calculator::OnLoaded(_In_ Object^, _In_ RoutedEventArgs^)
{
    Model->PropertyChanged += ref new PropertyChangedEventHandler(this, &Calculator::OnCalcPropertyChanged);
    Model->HideMemoryClicked += ref new HideMemoryClickedHandler(this, &Calculator::OnHideMemoryClicked);

    InitializeHistoryView(Model->HistoryVM);
    String^ historyPaneName = AppResourceProvider::GetInstance().GetResourceString(ref new String(m_historyPaneStr));
    HistoryFlyout->FlyoutPresenterStyle->Setters->Append(ref new Setter(AutomationProperties::NameProperty, historyPaneName));
    String^ memoryPaneName = AppResourceProvider::GetInstance().GetResourceString(ref new String(m_memoryPaneStr));
    MemoryFlyout->FlyoutPresenterStyle->Setters->Append(ref new Setter(AutomationProperties::NameProperty, memoryPaneName));

    if (Windows::Foundation::Metadata::ApiInformation::IsEventPresent(ref new String(m_windowsUIXamlControlsPrimitivesFlyoutBaseStr), ref new String(m_closingStr)))
    {
        HistoryFlyout->Closing += ref new TypedEventHandler<FlyoutBase^, FlyoutBaseClosingEventArgs^>(this, &Calculator::HistoryFlyout_Closing);
        MemoryFlyout->Closing += ref new TypedEventHandler<FlyoutBase^, FlyoutBaseClosingEventArgs^>(this, &Calculator::OnMemoryFlyoutClosing);
    }

    // Delay load things later when we get a chance.
    WeakReference weakThis(this);
    this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([weakThis]()
    {
        if (TraceLogger::GetInstance().UpdateWindowIdLog(ApplicationView::GetApplicationViewIdForWindow(CoreWindow::GetForCurrentThread())))
        {
            auto refThis = weakThis.Resolve<Calculator>();
            if (refThis != nullptr)
            {
                refThis->GetMemory();
            }
        }
    }));
}

std::wstring Calculator::GetCurrentLayoutState()
{
    if (IsProgrammer)
    {
        return m_programmerStr;
    }
    else if (IsScientific)
    {
        return m_scientificStr;
    }

    return m_standardStr;
}

void Calculator::UpdateViewState()
{
    const auto state = GetCurrentLayoutState();
    Model->IsDecimalEnabled = IsProgrammer ? false : true;

    CloseHistoryFlyout();
    CloseMemoryFlyout();

    VisualStateManager::GoToState(this, ref new String(state.c_str()), true/*useTransitions*/);
}

void Calculator::SetResultStyles()
{
    CoreWindow^ window = CoreWindow::GetForCurrentThread();
    if (window)
    {
        float curHeight = window->Bounds.Height;
        if (curHeight >= 800)
        {
            Results->Style = ResultsStyleL;
            RowResult->MinHeight = 108;
            RowResult->Height = GridLength(72, GridUnitType::Star);
        }
        else if ((IsProgrammer && curHeight >= 640) || (IsScientific && curHeight >= 544) || IsStandard)
        {
            Results->Style = ResultsStyleM;
            RowResult->MinHeight = 72;
            RowResult->Height = GridLength(72, GridUnitType::Star);
        }
        else
        {
            Results->Style = ResultsStyleS;
            RowResult->MinHeight = 42;
            RowResult->Height = GridLength(42, GridUnitType::Star);
        }
    }
}

void Calculator::AnimateCalculator(bool resultAnimate)
{
    if (App::IsAnimationEnabled())
    {
        m_doAnimate = true;
        m_resultAnimate = resultAnimate;
        if (((m_isLastAnimatedInScientific && IsScientific) || (!m_isLastAnimatedInScientific && !IsScientific)) && ((m_isLastAnimatedInProgrammer && IsProgrammer) || (!m_isLastAnimatedInProgrammer && !IsProgrammer)))
        {
            // We are forcing the animation here
            // It's because if last animation was in standard, then go to unit converter, then comes back to standard
            // The state for the calculator does not change and the animation would not get run.
            this->OnStoryboardCompleted(nullptr, nullptr);
        }
    }
}

void Calculator::OnContextRequested(UIElement^ sender, ContextRequestedEventArgs^ e)
{
    auto requestedElement = safe_cast<FrameworkElement^>(e->OriginalSource);

    PasteMenuItem->IsEnabled = CopyPasteManager::HasStringToPaste();

    Point point;
    if (e->TryGetPosition(requestedElement, &point))
    {
        m_displayFlyout->ShowAt(requestedElement, point);
    }
    else
    {
        // Not invoked via pointer, so let XAML choose a default location.
        m_displayFlyout->ShowAt(requestedElement);
    }

    e->Handled = true;
}

void Calculator::OnContextCanceled(UIElement^ sender, RoutedEventArgs^ e)
{
    m_displayFlyout->Hide();
}

void Calculator::OnLayoutStateChanged(_In_ Object^ sender, _In_ Object^ e)
{
    UpdateViewState();
    UpdatePanelViewState();
}

void Calculator::OnIsStandardPropertyChanged(bool /*oldValue*/, bool /*newValue*/)
{
    UpdateViewState();
    UpdatePanelViewState();
}

void Calculator::OnIsScientificPropertyChanged(bool /*oldValue*/, bool newValue)
{
    if (newValue)
    {
        EnsureScientific();
    }

    UpdateViewState();
    UpdatePanelViewState();
}

void Calculator::OnIsProgrammerPropertyChanged(bool /*oldValue*/, bool newValue)
{
    if (newValue)
    {
        EnsureProgrammer();
        m_pivotItem = static_cast<Windows::UI::Xaml::Controls::PivotItem^>(DockPivot->Items->GetAt(0));
        DockPivot->Items->RemoveAt(0);
    }
    else
    {
        if (m_pivotItem != nullptr && DockPivot->Items->Size == 1)
        {
            DockPivot->Items->InsertAt(0, m_pivotItem);
        }
    }

    DockPivot->SelectedIndex = 0;
    UpdateViewState();
    UpdatePanelViewState();
}

void Calculator::OnIsInErrorPropertyChanged()
{
    bool isError = Model->IsInError;

    String^ newState = isError ? ref new String(m_errorLayoutStr) : ref new String(m_noErrorLayoutStr);
    VisualStateManager::GoToState(this, newState, false);

    if (m_memory != nullptr)
    {
        m_memory->IsErrorVisualState = isError;
    }

    OpsPanel->IsErrorVisualState = isError;
    if (IsScientific && ScientificAngleButtons)
    {
        ScientificAngleButtons->IsErrorVisualState = isError;
    }
    else if (IsProgrammer && ProgrammerDisplayPanel)
    {
        ProgrammerDisplayPanel->IsErrorVisualState = isError;
    }
}

// Once the storyboard that rearranges the buttons completed,
// We do the animation based on the Mode or Orientation change.
void Calculator::OnStoryboardCompleted(_In_ Object^ sender, _In_ Object^ e)
{
    m_isLastAnimatedInScientific = IsScientific;
    m_isLastAnimatedInProgrammer = IsProgrammer;
    if (m_doAnimate)
    {
        m_doAnimate = false;
        if (m_resultAnimate)
        {
            m_resultAnimate = false;
            Animate->Begin();
        }
        else
        {
            AnimateWithoutResult->Begin();
        }
    }
    SetResultStyles();
}

void Calculator::EnsureScientific()
{
    OpsPanel->EnsureScientificOps();

    if (!ScientificAngleButtons)
    {
        this->FindName(ref new String(m_scientificAngleButtonsStr));
    }
}

void Calculator::EnsureProgrammer()
{
    if (!ProgrammerOperators)
    {
        this->FindName(ref new String(m_programmerOperatorsStr));
    }

    if (!ProgrammerDisplayPanel)
    {
        this->FindName(ref new String(m_programmerDisplayPanelStr));
    }

    OpsPanel->EnsureProgrammerRadixOps();
    ProgrammerOperators->SetRadixButton(Model->GetCurrentRadixType());
}

void Calculator::OnCalcPropertyChanged(_In_ Object^ sender, _In_ PropertyChangedEventArgs^ e)
{
    String^ prop = e->PropertyName;
    if (e->PropertyName->Equals(CalculatorViewModelProperties::IsMemoryEmpty))
    {
        UpdateMemoryState();
    }
    else if (e->PropertyName->Equals(CalculatorViewModelProperties::IsInError))
    {
        OnIsInErrorPropertyChanged();
    }
}

void Calculator::UpdatePanelViewState()
{
    UpdateHistoryState();
    UpdateMemoryState();
}

void Calculator::UpdateHistoryState()
{
    String^ viewState = App::GetAppViewState();
    if (viewState == ViewState::DockedView)
    {
        // docked view
        CloseHistoryFlyout();
        SetChildAsHistory();
        HistoryButton->Visibility = ::Visibility::Collapsed;

        if (m_IsLastFlyoutHistory)
        {
            DockPivot->SelectedIndex = 0;
        }
    }
    else
    {    // flyout view
        DockHistoryHolder->Child = nullptr;
        if (!IsProgrammer)
        {
            HistoryButton->Visibility = ::Visibility::Visible;
        }
    }
}

void Calculator::UpdateMemoryState()
{
    if (!Model->IsMemoryEmpty)
    {
        MemRecall->IsEnabled = true;
        ClearMemoryButton->IsEnabled = true;
    }
    else
    {
        MemRecall->IsEnabled = false;
        ClearMemoryButton->IsEnabled = false;
    }

    String^ viewState = App::GetAppViewState();
    if (viewState == ViewState::DockedView)
    {
        CloseMemoryFlyout();
        SetChildAsMemory();
        MemoryButton->Visibility = ::Visibility::Collapsed;

        if (m_IsLastFlyoutMemory && !IsProgrammer)
        {
            DockPivot->SelectedIndex = 1;
        }
    }
    else
    {
        MemoryButton->Visibility = ::Visibility::Visible;
        DockMemoryHolder->Child = nullptr;
    }
}

void Calculator::SetChildAsMemory()
{
    DockMemoryHolder->Child = GetMemory();
}

void Calculator::SetChildAsHistory()
{
    if (m_historyList == nullptr)
    {
        InitializeHistoryView(Model->HistoryVM);
    }

    DockHistoryHolder->Child = m_historyList;
}

void Calculator::OnHideHistoryClicked()
{
    ToggleHistoryFlyout(nullptr);
}

void Calculator::OnHistoryItemClicked(_In_ HistoryItemViewModel^ e)
{
    unsigned int tokenSize;
    assert(e->GetTokens() != nullptr);
    e->GetTokens()->GetSize(&tokenSize);
    TraceLogger::GetInstance().LogHistoryItemLoadBegin();
    Model->SetHistoryExpressionDisplay(e->GetTokens(), e->GetCommands());
    Model->SetExpressionDisplay(e->GetTokens(), e->GetCommands());
    Model->SetPrimaryDisplay(e->Result->Data(), false);
    Model->IsFToEEnabled = false;
   
    TraceLogger::GetInstance().LogHistoryItemLoadEnd(tokenSize);
    CloseHistoryFlyout();
    this->Focus(::FocusState::Programmatic);
}

void Calculator::HistoryFlyout_Opened(_In_ Object ^sender, _In_ Object ^args)
{
    m_fIsHistoryFlyoutOpen = true;
    m_IsLastFlyoutMemory = false;
    m_IsLastFlyoutHistory = true;
    EnableControls(false);
    AutomationProperties::SetName(HistoryButton, m_closeHistoryFlyoutAutomationName);
    TraceLogger::GetInstance().LogHistoryFlyoutOpenEnd(Model->HistoryVM->ItemSize);
    TraceLogger::GetInstance().LogHistoryBodyOpened();
}

void Calculator::HistoryFlyout_Closing(_In_ FlyoutBase^ sender, _In_ FlyoutBaseClosingEventArgs^ args)
{
    // Set in the Closing event so the new name is available when the Flyout has Closed.
    AutomationProperties::SetName(HistoryButton, m_openHistoryFlyoutAutomationName);
}

void Calculator::HistoryFlyout_Closed(_In_ Object ^sender, _In_ Object ^args)
{
    // Ideally, this would be renamed in the Closing event because the Closed event is too late.
    // Closing is not available until RS1+ so we set the name again here for TH2 support.
    AutomationProperties::SetName(HistoryButton, m_openHistoryFlyoutAutomationName);
    m_fIsHistoryFlyoutOpen = false;
    EnableControls(true);
    if (HistoryButton->IsEnabled)
    {
        HistoryButton->Focus(::FocusState::Programmatic);
    }

    FullscreenFlyoutClosed();
}

void Calculator::CloseHistoryFlyout()
{
    if (m_fIsHistoryFlyoutOpen)
    {
        HistoryFlyout->Hide();
    }
}

void Calculator::CloseMemoryFlyout()
{
    if (m_fIsMemoryFlyoutOpen)
    {
        MemoryFlyout->Hide();
    }
}

void Calculator::SetDefaultFocus()
{
    Results->Focus(::FocusState::Programmatic);
}

void Calculator::ToggleHistoryFlyout(Object^ /*parameter*/)
{
    String^ viewState = App::GetAppViewState();
    // If app starts correctly in snap mode and shortcut is used for history then we need to load history if not yet initialized.
    if (viewState != ViewState::DockedView)
    {
        if (m_fIsHistoryFlyoutOpen)
        {
            HistoryFlyout->Hide();
        }
        else
        {
            TraceLogger::GetInstance().LogHistoryFlyoutOpenBegin(Model->HistoryVM->ItemSize);
            HistoryFlyout->Content = m_historyList;
            m_historyList->RowHeight = NumpadPanel->ActualHeight;
            FlyoutBase::ShowAttachedFlyout(HistoryButton);
        }
    }
}

void Calculator::ToggleMemoryFlyout()
{
    String^ viewState = App::GetAppViewState();
    if (viewState != ViewState::DockedView)
    {
        if (m_fIsMemoryFlyoutOpen)
        {
            MemoryFlyout->Hide();
        }
        else
        {
            TraceLogger::GetInstance().LogMemoryFlyoutOpenBegin(Model->MemorizedNumbers->Size);
            MemoryFlyout->Content = GetMemory();
            m_memory->RowHeight = NumpadPanel->ActualHeight;
            FlyoutBase::ShowAttachedFlyout(MemoryButton);
        }
    }
}

void Calculator::OnMemoryFlyoutOpened(_In_ Object ^sender, _In_ Object ^args)
{
    TraceLogger::GetInstance().LogMemoryFlyoutOpenEnd(Model->MemorizedNumbers->Size);
    m_IsLastFlyoutMemory = true;
    m_IsLastFlyoutHistory = false;
    m_fIsMemoryFlyoutOpen = true;
    AutomationProperties::SetName(MemoryButton, m_closeMemoryFlyoutAutomationName);
    EnableControls(false);
    TraceLogger::GetInstance().LogMemoryBodyOpened();
}

void Calculator::OnMemoryFlyoutClosing(_In_ FlyoutBase^ sender, _In_ FlyoutBaseClosingEventArgs^ args)
{
    // Set in the Closing event so the new name is available when the Flyout has Closed.
    AutomationProperties::SetName(MemoryButton, m_openMemoryFlyoutAutomationName);
}

void Calculator::OnMemoryFlyoutClosed(_In_ Object ^sender, _In_ Object ^args)
{
    // Ideally, this would be renamed in the Closing event because the Closed event is too late.
    // Closing is not available until RS1+ so we set the name again here for TH2 support.
    AutomationProperties::SetName(MemoryButton, m_openMemoryFlyoutAutomationName);
    m_fIsMemoryFlyoutOpen = false;
    EnableControls(true);
    if (MemoryButton->IsEnabled)
    {
        MemoryButton->Focus(::FocusState::Programmatic);
    }

    FullscreenFlyoutClosed();
}

Memory^ Calculator::GetMemory()
{
    if (m_memory == nullptr)
    {
        m_memory = ref new Memory();
        VisualStateManager::GoToState(m_memory, ref new String(GetCurrentLayoutState().c_str()), true/*useTransitions*/);
    }

    return m_memory;
}

void Calculator::OnHideMemoryClicked()
{
    if (!m_fIsMemoryFlyoutOpen)
    {
        this->Focus(::FocusState::Programmatic);
    }

    MemoryFlyout->Hide();
}

void Calculator::EnableMemoryControls(bool enable)
{
    memButton->IsEnabled = enable;
    MemMinus->IsEnabled = enable;
    MemPlus->IsEnabled = enable;
    if (!Model->IsMemoryEmpty)
    {
        MemRecall->IsEnabled = enable;
        ClearMemoryButton->IsEnabled = enable;
    }
}

void Calculator::EnableControls(bool enable)
{
    OpsPanel->IsEnabled = enable;
    
    EnableMemoryControls(enable);
}

void Calculator::OnMemoryFlyOutTapped(_In_ Object^ sender, _In_ TappedRoutedEventArgs^ e)
{
    Grid^ grid = safe_cast<Grid^> (sender);
    Point point = e->GetPosition(nullptr);

    if (point.Y < (grid->ActualHeight - NumpadPanel->ActualHeight))
    {
        MemoryFlyout->Hide();
    }
}


void Calculator::OnHistoryFlyOutTapped(_In_ Object^ sender, _In_ TappedRoutedEventArgs^ e)
{
    Grid^ grid = safe_cast<Grid^> (sender);
    Point point = e->GetPosition(nullptr);

    if (point.Y < (grid->ActualHeight - NumpadPanel->ActualHeight))
    {
        HistoryFlyout->Hide();
    }
}

void Calculator::expressionContainer_LayoutUpdated(_In_ Object^ sender, _In_ Object^ e)
{
    expressionText->UpdateScrollButtons();
}

bool Calculator::IsValidRegularExpression(std::wstring str)
{
    bool result = false;
    std::wregex regexPatterns[3];
    regexPatterns[0] = m_validRegexPatternZeroStr;
    regexPatterns[1] = m_validRegexPatternOneStr;
    regexPatterns[2] = m_validRegexPatternTwoStr;

    const auto& localizer = LocalizationSettings::GetInstance();
    String^ englishString = localizer.GetEnglishValueFromLocalizedDigits(str);

    for (int i = 0; i < 3; ++i)
    {
        if (regex_match(englishString->Data(), regexPatterns[i]))
        {
            result = true;
            break;
        }
    }
    return result;
}

void Calculator::DockPanelTapped(_In_ TappedRoutedEventArgs^ e)
{
    int index = DockPivot->SelectedIndex;
    if (index == 1 && !IsProgrammer)
    {
        SetChildAsMemory();
    }

    m_IsLastFlyoutMemory = false;
    m_IsLastFlyoutHistory = false;
}

void Calculator::OnResultsLayoutChanged(_In_ Object^ sender, _In_ Object^ e)
{
    SetResultStyles();
}

void Calculator::UnregisterEventHandlers()
{
    expressionText->UnregisterEventHandlers();
}

void Calculator::OnErrorLayoutCompleted(_In_ Object^ sender, _In_ Object^ e)
{
    SetDefaultFocus();
}

void Calculator::OnHistoryAccessKeyInvoked(_In_ UIElement^ sender, _In_ AccessKeyInvokedEventArgs^ args)
{
    DockPivot->SelectedItem = HistoryPivotItem;
}

void Calculator::OnMemoryAccessKeyInvoked(_In_ UIElement^ sender, _In_ AccessKeyInvokedEventArgs^ args)
{
    DockPivot->SelectedItem = MemoryPivotItem;
}


void CalculatorApp::Calculator::DockPivot_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
    if (DockPivot->SelectedIndex == 0)
    {
        TraceLogger::GetInstance().LogHistoryBodyOpened();
    }
    else
    {
        TraceLogger::GetInstance().LogMemoryBodyOpened();
    }
}
