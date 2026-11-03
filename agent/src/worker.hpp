#pragma once
#include "telemetry.pb.h"

#include <windows.h>
#include <filesystem>
#include <cstdint>
#include <string>

namespace tel = cplussharpqueue::telemetry;

class Worker
{
public:
    explicit Worker(std::filesystem::path outputDir);

    /**
     * @brief Collects memory data until stopEvent is signaled, then flushes.
     * 
     * @param stopEvent signal to stop event
     * @param outputDir where everything is stored
     * @return false on I/O failure
     */
    bool run(HANDLE stopEvent);

private:
    std::uint64_t time_now_ms();
    std::string computer_name();
    bool read_memory(std::uint64_t& used, std::uint64_t& total);

    void start_batch();
    bool write_batch();

    std::filesystem::path outputDir_;
    std::string host_;
    std::uint64_t sequence_ = 0;
    cplussharpqueue::telemetry::Batch batch_;
};