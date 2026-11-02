#include "telemetry.pb.h"
#include <google/protobuf/util/message_differencer.h>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

namespace tel = cplussharpqueue::telemetry;

namespace {

std::uint64_t time_now_ms()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

tel::Batch make_batch(std::uint32_t sampleQty)
{
    tel::Batch batch;
    batch.set_host("dev-machine");
    batch.set_architecture(tel::ARCHITECTURE_X64);
    batch.set_sequence(1);
    batch.set_mem_total_bytes(16ull * 1024 * 1024 * 1024);

    for (std::uint8_t itr = 0; itr < sampleQty; itr++)
    {
        tel::Sample* sample = batch.add_samples();
        sample->set_timestamp_unix_ms(time_now_ms());
        sample->set_cpu_percent(20.0f + static_cast<float>(itr));
        sample->set_mem_used_bytes((8ull + itr) * 1024 * 1024 * 1024);
    }

    return batch;
}

bool run()
{
    const tel::Batch initialBatch = make_batch(2);

    std::string bytes;
    if (!initialBatch.SerializeToString(&bytes))
    {
        std::cerr << "Failed to serialize" << std::endl;
        return false;
    }
    std::cout << "Serialized batch: " << bytes.size() << "bytes." << std::endl;

    tel::Batch parsedBatch;
    if (!parsedBatch.ParseFromString(bytes))
    {
        std::cerr << "Failed to parse" << std::endl;
        return false;
    }

    const bool bothAreEqual =
        google::protobuf::util::MessageDifferencer::Equals(initialBatch, parsedBatch);
    std::cout << "Roundtrip is " << (bothAreEqual ? "OK" : "NOT OK") << std::endl;
    std::cout << parsedBatch.DebugString();

    std::filesystem::create_directories("data");
    std::ofstream out("data/batch.bin", std::ios::binary);
    if (!out)
    {
        std::cerr << "Failed to open data/batch.bin" << std::endl;
        return false;
    }

    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    std::cout << "Stored in data/batch.bin" << std::endl;

    return bothAreEqual;
}

}

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    bool runResult = run();
    google::protobuf::ShutdownProtobufLibrary();
    
    return runResult ? 0 : 1;
}