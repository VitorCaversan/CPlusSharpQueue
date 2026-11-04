#include "worker.hpp"
#include "service.hpp"

#include <cstdio>
#include <exception>
#include <filesystem>
#include <system_error>

namespace {
    constexpr wchar_t kServiceName[] = L"CPlusSharpQueue";

    std::filesystem::path service_output_dir()
    {
        wchar_t buffer[MAX_PATH];
        const DWORD n = GetEnvironmentVariableW(L"ProgramData", buffer, MAX_PATH);
        const std::filesystem::path base =
            (n > 0 && n < MAX_PATH) ? std::filesystem::path(buffer) : std::filesystem::path(L"C:\\ProgramData");
        return base / L"CPlusSharpQueue";
    }

    void log_event(WORD type, const wchar_t* text)
    {
        HANDLE source = RegisterEventSourceW(nullptr, kServiceName);
        if (!source)
            return;
        const wchar_t* strings[] = { text };
        ReportEventW(source, type, 0, 0, nullptr, 1, 0, strings, nullptr);
        DeregisterEventSource(source);
    }
}

// RAII Lifetime mangement

Service::Service()
    : stopEvent_(CreateEventW(nullptr, TRUE, FALSE, nullptr))  // manual reset, starts unsignaled
{
    if (!stopEvent_)
        throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "CreateEventW");
    instance_ = this;
}

Service::~Service()
{
    instance_ = nullptr;
    CloseHandle(stopEvent_);
}

// Public entrypoints

int Service::run_as_service()
{
    SERVICE_TABLE_ENTRYW table[] = {
        { const_cast<LPWSTR>(kServiceName), &Service::service_main },
        { nullptr, nullptr },
    };

    if (!StartServiceCtrlDispatcherW(table))
    {
        if (GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
            std::fwprintf(stderr, L"Not started by the SCM. Use --console to run interactively.\n");
        return 1;
    }
    return 0;
}

int Service::run_in_console()
{
    SetConsoleCtrlHandler(&Service::console_handler, TRUE);
    std::fwprintf(stdout, L"Running in console, writing to .\\data. Ctrl+C to stop.\n");

    Worker worker(L"data");
    const bool ok = worker.run(stopEvent_);

    SetConsoleCtrlHandler(&Service::console_handler, FALSE);
    return ok ? 0 : 1;
}

// ---------- static trampolines ----------

void WINAPI Service::service_main(DWORD, LPWSTR*)
{
    instance_->on_service_main();
}

DWORD WINAPI Service::control_handler(DWORD control, DWORD, LPVOID, LPVOID context)
{
    return static_cast<Service*>(context)->on_control(control);
}

BOOL WINAPI Service::console_handler(DWORD type)
{
    if (type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT || type == CTRL_CLOSE_EVENT)
    {
        SetEvent(instance_->stopEvent_);
        return TRUE;
    }
    return FALSE;
}

// SERVICE LOGIC

void Service::on_service_main()
{
    statusHandle_ = RegisterServiceCtrlHandlerExW(kServiceName, &Service::control_handler, this);
    if (!statusHandle_)
        return;

    report_status(SERVICE_START_PENDING, NO_ERROR, 3000);

    bool ok = false;
    try
    {
        Worker worker(service_output_dir());  // initialization happens while START_PENDING
        report_status(SERVICE_RUNNING);
        log_event(EVENTLOG_INFORMATION_TYPE, L"Started");
        ok = worker.run(stopEvent_);
    }
    catch (const std::exception&)
    {
        // An exception must never cross back into the SCM's C code.
    }

    if (!ok)
        status_.dwServiceSpecificExitCode = 1;
    report_status(SERVICE_STOPPED, ok ? NO_ERROR : ERROR_SERVICE_SPECIFIC_ERROR);
    log_event(ok ? EVENTLOG_INFORMATION_TYPE : EVENTLOG_ERROR_TYPE, L"Started");
}

DWORD Service::on_control(DWORD control)
{
    switch (control)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        report_status(SERVICE_STOP_PENDING, NO_ERROR, 5000);
        SetEvent(stopEvent_);  // the real work happens in on_service_main
        return NO_ERROR;
    case SERVICE_CONTROL_INTERROGATE:
        return NO_ERROR;
    default:
        return ERROR_CALL_NOT_IMPLEMENTED;
    }
}

void Service::report_status(DWORD state, DWORD exitCode, DWORD waitHintMs)
{
    status_.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status_.dwCurrentState = state;
    status_.dwWin32ExitCode = exitCode;
    status_.dwWaitHint = waitHintMs;
    status_.dwControlsAccepted =
        (state == SERVICE_RUNNING) ? (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN) : 0;
    status_.dwCheckPoint =
        (state == SERVICE_RUNNING || state == SERVICE_STOPPED) ? 0 : checkPoint_++;

    SetServiceStatus(statusHandle_, &status_);
}