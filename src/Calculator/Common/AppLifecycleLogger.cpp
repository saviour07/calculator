// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include "AppLifecycleLogger.h"
#include "CalcViewModel/Common/TraceActivity.h"

using namespace std;
using namespace winrt;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Background;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Foundation::Diagnostics;
using namespace winrt::Windows::UI::ViewManagement;

namespace CalculatorApp
{

#ifdef SEND_TELEMETRY
    // c.f. WINEVENT_KEYWORD_RESERVED_63-56 0xFF00000000000000 // Bits 63-56 - channel keywords
    // c.f. WINEVENT_KEYWORD_*              0x00FF000000000000 // Bits 55-48 - system-reserved keywords
    constexpr int64_t MICROSOFT_KEYWORD_CRITICAL_DATA = 0x0000800000000000; // Bit 47
    constexpr int64_t MICROSOFT_KEYWORD_MEASURES = 0x0000400000000000; // Bit 46
    constexpr int64_t MICROSOFT_KEYWORD_TELEMETRY = 0x0000200000000000; // Bit 45
    constexpr int64_t MICROSOFT_KEYWORD_RESERVED_44 = 0x0000100000000000; // Bit 44 (reserved for future assignment)
#else
    // define all Keyword options as 0 when we do not want to upload app telemetry
    constexpr int64_t MICROSOFT_KEYWORD_CRITICAL_DATA = 0;
    constexpr int64_t MICROSOFT_KEYWORD_MEASURES = 0;
    constexpr int64_t MICROSOFT_KEYWORD_TELEMETRY = 0;
    constexpr int64_t MICROSOFT_KEYWORD_RESERVED_44 = 0;
#endif

#pragma region TraceLogger setup and cleanup

    AppLifecycleLogger::AppLifecycleLogger() :
        m_appLifecycleProvider(
            m_microsoftWindowsAppLifeCycleStr,
            LoggingChannelOptions(GUID{ 0x4f50731a, 0x89cf, 0x4782, 0xb3, 0xe0, 0xdc, 0xe8, 0xc9, 0x4, 0x76, 0xba }), // Microsoft Telemetry group
            GUID{ 0xef00584a, 0x2655, 0x462c, 0xbc, 0x24, 0xe7, 0xde, 0x63, 0xe, 0x7f, 0xbf }) //Unique provider ID {EF00584A-2655-462C-BC24-E7DE630E7FBF}
    {
    }

    AppLifecycleLogger::~AppLifecycleLogger()
    {
    }

    AppLifecycleLogger& AppLifecycleLogger::GetInstance()
    {
        static AppLifecycleLogger s_selfInstance;
        return s_selfInstance;
    }

    bool AppLifecycleLogger::GetTraceLoggingProviderEnabled() const
    {
        return m_appLifecycleProvider.Enabled();
    }

#pragma region Tracing methods
    void AppLifecycleLogger::LogAppLifecycleEvent(hstring const& eventName, LoggingFields const& fields) const
    {
        m_appLifecycleProvider.LogEvent(eventName, fields, LoggingLevel::Information, LoggingOptions(MICROSOFT_KEYWORD_TELEMETRY | WINEVENT_KEYWORD_RESPONSE_TIME));
    }
#pragma endregion

    void AppLifecycleLogger::LaunchUIResponsive() const
    {
        if (!GetTraceLoggingProviderEnabled()) return;

        LoggingFields fields{};
        PopulateAppInfo(fields);
        LogAppLifecycleEvent(m_modernAppLaunchUIResponsiveStr, fields);
    }

    void AppLifecycleLogger::LaunchVisibleComplete() const
    {
        if (!GetTraceLoggingProviderEnabled()) return;

        LoggingFields fields{};
        PopulateAppInfo(fields);
        LogAppLifecycleEvent(m_modernAppLaunchVisibleCompleteStr, fields);
    }

    void AppLifecycleLogger::ResumeUIResponsive() const
    {
        if (!GetTraceLoggingProviderEnabled()) return;

        LoggingFields fields{};
        PopulateAppInfo(fields);
        LogAppLifecycleEvent(m_modernAppResumeUIResponsiveStr, fields);
    }

    void AppLifecycleLogger::ResumeVisibleComplete() const
    {
        if (!GetTraceLoggingProviderEnabled()) return;

        LoggingFields fields{};
        PopulateAppInfo(fields);
        LogAppLifecycleEvent(m_modernAppResumeVisibleCompleteStr, fields);
    }

    void AppLifecycleLogger::ResizeUIResponsive() const
    {
        ResizeUIResponsive(ApplicationView::GetForCurrentView().Id());
    }

    void AppLifecycleLogger::ResizeUIResponsive(int32_t viewId) const
    {
        if (!GetTraceLoggingProviderEnabled()) return;

        LoggingFields fields{};
        PopulateAppInfo(fields);
        fields.AddInt32(m_ViewIdStr, viewId);
        LogAppLifecycleEvent(m_modernAppResizeUIResponsiveStr, fields);
    }

    void AppLifecycleLogger::ResizeVisibleComplete() const
    {
        ResizeVisibleComplete(ApplicationView::GetForCurrentView().Id());
    }

    void AppLifecycleLogger::ResizeVisibleComplete(int32_t viewId) const
    {
        if (!GetTraceLoggingProviderEnabled()) return;

        LoggingFields fields{};
        PopulateAppInfo(fields);
        fields.AddInt32(m_ViewIdStr, viewId);
        LogAppLifecycleEvent(m_modernAppResizeVisibleCompleteStr, fields);
    }

    void AppLifecycleLogger::PopulateAppInfo(LoggingFields& fields) const
    {
        auto appId = CoreApplication::Id();
        auto aumId = Package::Current().Id().FamilyName() + m_exclamationStr + appId;
        auto packageFullName = Package::Current().Id().FullName();
        auto psmKey = Package::Current().Id().FullName() + m_additionStr + appId;

        fields.AddString(m_aumIdStr, aumId);
        fields.AddString(m_packageFullNameStr, packageFullName);
        fields.AddString(m_psmKeyStr, psmKey);
    }
}



