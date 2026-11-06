#pragma once
#include <atomic>
#include <cstdio>
#include <memory>
#include "commonMemory.hpp"
#include <filesystem>

struct RecordHeader_S
{
    char     identifier[4];
    uint32_t sampleSize;
    uint32_t reserved;
};

class Recorder
{
  public:
    Recorder(std::shared_ptr<commonMemory_S> data) : m_data(data)
    {
    }
    void recordLoop(std::atomic<bool>& isRunning);

  private:
    FILE*                           openNewRecording();
    std::shared_ptr<commonMemory_S> m_data;
};

bool exportToCsv(const std::string& binPath, const std::string& csvPath);