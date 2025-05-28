#pragma once
#include "CLI/CLI.hpp"
#include "candle.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "MD.hpp"
#include <algorithm>
#include <memory>
#include <candle_types.hpp>

namespace mab
{

    class MDCli
    {
      public:
        MDCli() = delete;
        MDCli(CLI::App* rootCli, const std::shared_ptr<const CandleBuilder> candleBuilder);
        ~MDCli() = default;

      private:
        Logger m_logger = Logger(Logger::ProgramLayer_E::TOP, "MD_CLI");
        std::unique_ptr<MD, std::function<void(MD*)>> getMd(
            const std::shared_ptr<canId_t>             mdCanId,
            const std::shared_ptr<const CandleBuilder> candleBuilder);
    };
}  // namespace mab
