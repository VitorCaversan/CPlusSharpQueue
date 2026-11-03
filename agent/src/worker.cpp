#include "worker.hpp"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>

namespace tel = cplussharpqueue::telemetry;

namespace {
    constexpr DWORD kSampleIntervalMs = 1000;
    constexpr int kSamplesPerBatch = 10;

    constexpr tel::Architecture build_architecture()
    {
#if defined(_M_ARM64)
        return tel::ARCHITECTURE_ARM64;
#elif defined(_M_X64)
        return tel::ARCHITECTURE_X64;
#else
        return tel::ARCHITECTURE_UNSPECIFIED;
#endif
    }
}

Worker::Worker(std::filesystem::path outputDir)
    : outputDir_(std::move(outputDir)), host_(computer_name())
{
}

std::uint64_t Worker::time_now_ms()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

std::string Worker::computer_name()
{
    wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = static_cast<DWORD>(std::size(buffer));

    if (!GetComputerNameW(buffer, &size))
        return "unknown";

    const int length = WideCharToMultiByte(CP_UTF8, 0, buffer, static_cast<int>(size),
                                           nullptr, 0, nullptr, nullptr);
    std::string resultingString(static_cast<size_t>(length), '\0');
    WideCharToMultiByte(CP_UTF8, 0, buffer, static_cast<int>(size),
                        resultingString.data(), length, nullptr, nullptr);

    return resultingString;
}

bool Worker::read_memory(std::uint64_t& used, std::uint64_t& total)
{
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status))
        return false;
    total = status.ullTotalPhys;
    used = status.ullTotalPhys - status.ullAvailPhys;
    return true;
}

void Worker::start_batch()
{
    batch_.Clear();
    batch_.set_host(host_);
    batch_.set_architecture(build_architecture());
    batch_.set_sequence(++sequence_);
}

bool Worker::write_batch()
{
    const auto finalPath = outputDir_ / L"latest.bin";
    const auto tempPath = outputDir_ / L"latest.bin.tmp";
    {
        std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
        if (!out || !batch_.SerializeToOstream(&out))
            return false;
    }
    return MoveFileExW(tempPath.c_str(), finalPath.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
}

bool Worker::run(HANDLE stopEvent)
{
    std::error_code ec;
    std::filesystem::create_directories(outputDir_, ec);

    start_batch();

    while (WaitForSingleObject(stopEvent, kSampleIntervalMs) == WAIT_TIMEOUT)
    {
        std::uint64_t used = 0;
        std::uint64_t total = 0;
        if (read_memory(used, total))
        {
            batch_.set_mem_total_bytes(total);
            tel::Sample* sample = batch_.add_samples();
            sample->set_timestamp_unix_ms(time_now_ms());
            sample->set_mem_used_bytes(used);
        }

        if (batch_.samples_size() >= kSamplesPerBatch)
        {
            if (!write_batch())
                return false;
            start_batch();
        }
    }

    // Stop requested: flush what we have.
    return batch_.samples_size() == 0 || write_batch();
}
