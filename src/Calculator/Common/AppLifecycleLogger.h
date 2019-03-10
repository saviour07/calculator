// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

namespace CalculatorApp
{
    class AppLifecycleLogger
    {
    public:
        AppLifecycleLogger(AppLifecycleLogger const&) = delete;
        AppLifecycleLogger const & operator= (AppLifecycleLogger const&) = delete;
        ~AppLifecycleLogger();
        static AppLifecycleLogger& GetInstance();
        bool GetTraceLoggingProviderEnabled() const;

        void LaunchUIResponsive() const;
        void LaunchVisibleComplete() const;
        void ResumeUIResponsive() const;
        void ResumeVisibleComplete() const;

        void ResizeUIResponsive() const;
        void ResizeVisibleComplete() const;
        void ResizeUIResponsive(int32_t viewId) const;
        void ResizeVisibleComplete(int32_t viewId) const;

    private:
        // Make the object construction private to allow singleton access to this class
        AppLifecycleLogger();

        // Any new Log method should
        // a) decide the level of logging. This will help us in limiting recording of events only up to a certain level. See this link for guidance https://msdn.microsoft.com/en-us/library/windows/desktop/aa363742(v=vs.85).aspx
        // We're using Verbose level for events that are called frequently and needed only for debugging or capturing perf for specific scenarios 
        // b) should decide whether or not to log to  telemetry and pass TraceLoggingKeyword(MICROSOFT_KEYWORD_TELEMETRY) accordingly
        // c) Should accept a variable number of additional data arguments if needed
        void LogAppLifecycleEvent(winrt::hstring const& eventName, winrt::Windows::Foundation::Diagnostics::LoggingFields const& fields) const;
        void PopulateAppInfo(winrt::Windows::Foundation::Diagnostics::LoggingFields& fields) const;

        winrt::Windows::Foundation::Diagnostics::LoggingChannel m_appLifecycleProvider;

    private:
        static constexpr auto m_microsoftWindowsAppLifeCycleStr = L"Microsoft.Windows.AppLifeCycle";
        static constexpr auto m_modernAppLaunchUIResponsiveStr = L"ModernAppLaunch_UIResponsive";
        static constexpr auto m_modernAppLaunchVisibleCompleteStr = L"ModernAppLaunch_VisibleComplete";
        static constexpr auto m_modernAppResumeUIResponsiveStr = L"ModernAppResume_UIResponsive";
        static constexpr auto m_modernAppResumeVisibleCompleteStr = L"ModernAppResume_VisibleComplete";
        static constexpr auto m_ViewIdStr = L"ViewId";
        static constexpr auto m_modernAppResizeUIResponsiveStr = L"ModernAppResize_UIResponsive";
        static constexpr auto m_modernAppResizeVisibleCompleteStr = L"ModernAppResize_VisibleComplete";
        static constexpr auto m_exclamationStr = L"!";
        static constexpr auto m_additionStr = L"+";
        static constexpr auto m_aumIdStr = L"AumId";
        static constexpr auto m_packageFullNameStr = L"PackageFullName";
        static constexpr auto m_psmKeyStr = L"PsmKey";
    };
}
