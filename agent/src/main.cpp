#include "telemetry.pb.h"
#include "service.hpp"

#include <cstdio>
#include <cwchar>
#include <exception>

int wmain(int argc, wchar_t* argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    int result = 1;
    try
    {
        Service service;
        const bool console = argc > 1 && std::wcscmp(argv[1], L"--console") == 0;
        result = console ? service.run_in_console() : service.run_as_service();
    }
    catch (const std::exception& e)
    {
        std::fprintf(stderr, "Fatal: %s\n", e.what());
    }

    google::protobuf::ShutdownProtobufLibrary();
    return result;
}