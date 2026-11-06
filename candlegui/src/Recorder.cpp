#include "Recorder.hpp"
#include <cstdint>
#include <ctime>

void Recorder::recordLoop(std::atomic<bool>& isRunning)
{
    FILE*                       file = nullptr;
    std::vector<RecordSample_S> batch;
    batch.reserve(4096);
    int flushCounter = 0;

    while (isRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        const bool isRecording = m_data->recording.load();
        if (isRecording && file == nullptr)
            file = openNewRecording();

        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            batch.swap(m_data->recPending);
        }

        if (file != nullptr && !batch.empty())
        {
            fwrite(batch.data(), sizeof(RecordSample_S), batch.size(), file);
            m_data->recSamplesWritten += batch.size();
        }
        batch.clear();

        if (file != nullptr && ++flushCounter >= 10)
        {
            fflush(file);
            flushCounter = 0;
        }

        if (!isRecording && file != nullptr)
        {
            fclose(file);
            file = nullptr;
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_data->mtx);
        batch.swap(m_data->recPending);
    }

    if (file != nullptr)
    {
        fwrite(batch.data(), sizeof(RecordSample_S), batch.size(), file);
        fclose(file);
    }
}

FILE* Recorder::openNewRecording()
{
    std::filesystem::create_directories("recordings");

    std::time_t now = std::time(nullptr);
    char        name[64];
    std::strftime(name, sizeof(name), "recordings/rec_%Y-%m-%d_%H-%M-%S.bin", std::localtime(&now));

    FILE* file = fopen(name, "wb");
    if (file == nullptr)
        return nullptr;

    RecordHeader_S header = {{'M', 'D', 'R', 'C'}, sizeof(RecordSample_S), 0};
    fwrite(&header, sizeof(header), 1, file);

    m_data->recSamplesWritten = 0;
    return file;
}

bool exportToCsv(const std::string& binPath, const std::string& csvPath)
{
    FILE* inputFile = fopen(binPath.c_str(), "rb");
    if (inputFile == nullptr)
        return false;

    RecordHeader_S header;
    if (fread(&header, sizeof(header), 1, inputFile) != 1 ||
        memcmp(header.identifier, "MDRC", 4) != 0 || header.sampleSize != sizeof(RecordSample_S))
    {
        fclose(inputFile);
        return false;
    }

    std::map<uint32_t, FILE*> outFiles;
    bool                      failed = false;

    std::vector<RecordSample_S> batch(4096);
    size_t                      n;
    while (!failed &&
           (n = fread(batch.data(), sizeof(RecordSample_S), batch.size(), inputFile)) > 0)
    {
        for (size_t i = 0; i < n; ++i)
        {
            const RecordSample_S& sample = batch[i];

            FILE*& out = outFiles[sample.id];
            if (out == nullptr)
            {
                std::string path = csvPath + "_MD" + std::to_string(sample.id) + ".csv";
                out              = fopen(path.c_str(), "w");
                if (out == nullptr)
                {
                    failed = true;
                    break;
                }
                fprintf(out, "time_s,position,velocity,torque,mosfet_temp\n");
            }

            fprintf(out,
                    "%.6f,%.6f,%.6f,%.6f,%.2f\n",
                    sample.time,
                    sample.position,
                    sample.velocity,
                    sample.torque,
                    sample.mosfetTemp);
        }
    }

    for (const auto& [id, file] : outFiles)
    {
        if (file != nullptr)
            fclose(file);
    }
    fclose(inputFile);
    return !failed;
}
