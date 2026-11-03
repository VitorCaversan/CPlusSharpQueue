#pragma once
#include <windows.h>

class Service
{
public:
    Service();
    ~Service();

    // Owns a kernel handle: copying would close it twice.
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;

    int run_as_service();
    int run_in_console();

private:
    // Windows callbacks
    static void WINAPI service_main(DWORD argc, LPWSTR* argv);
    static DWORD WINAPI control_handler(DWORD control, DWORD eventType, LPVOID eventData, LPVOID context);
    static BOOL WINAPI console_handler(DWORD type);

    void on_service_main();
    DWORD on_control(DWORD control);
    void report_status(DWORD state, DWORD exitCode = NO_ERROR, DWORD waitHintMs = 0);

    // ServiceMain and the console handler receive no context parameter, so we use this static to
    // keep the current context for method callbacks on these handlers
    inline static Service* instance_ = nullptr;

    HANDLE stopEvent_ = nullptr;
    SERVICE_STATUS_HANDLE statusHandle_ = nullptr;
    SERVICE_STATUS status_{};
    DWORD checkPoint_ = 1;
};
