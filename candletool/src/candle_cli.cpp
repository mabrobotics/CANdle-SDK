#include "candle_cli.hpp"
#include "mabFileParser.hpp"
#include "candle_bootloader.hpp"
#include "mab_crc.hpp"

namespace mab
{
    CandleCli::CandleCli(CLI::App*                                  rootCli,
                         const std::shared_ptr<const CandleBuilder> candleBuilder)
    {
        auto* candleCli =
            rootCli->add_subcommand("candle", "CANdle device commands.")->require_subcommand();
        // Update
        auto* update = candleCli->add_subcommand("update", "Update Candle firmware.");

        UpdateOptions updateOptions(update);
        update->callback(
            [this, updateOptions]()
            {
                m_logger.info("Performing Candle firmware update.");

                MabFileParser candleFirmware(*updateOptions.pathToMabFile,
                                             MabFileParser::TargetDevice_E::CANDLE);

                auto candle_bootloader = attachCandleBootloader();
                for (size_t i = 0; i < candleFirmware.m_fwEntry.size;
                     i += CandleBootloader::PAGE_SIZE_STM32G474)
                {
                    std::array<u8, CandleBootloader::PAGE_SIZE_STM32G474> page;
                    std::memcpy(
                        page.data(), &candleFirmware.m_fwEntry.data->data()[i], page.size());
                    u32 crc = candleCRC::crc32(page.data(), page.size());
                    if (candle_bootloader->writePage(page, crc) != candleTypes::Error_t::OK)
                    {
                        m_logger.error("Candle flashing failed!");
                        break;
                    }
                }
            });
        // Version
        auto* version = candleCli->add_subcommand("version", "Get CANdle device version.");
        version->callback(
            [this, candleBuilder]()
            {
                auto candle = candleBuilder->build();
                if (!candle.has_value())
                {
                    m_logger.error("Could not connect to CANdle!");
                    return;
                }

                auto versionOpt = candle.value()->getCandleVersion();
                if (!versionOpt.has_value())
                {
                    m_logger.error("Could not read CANdle version!");
                    return;
                }

                m_logger.info("CANdle firmware version: %d.%d.%d(%c)",
                              versionOpt.value().s.major,
                              versionOpt.value().s.minor,
                              versionOpt.value().s.revision,
                              versionOpt.value().s.tag);
            });
    }
}  // namespace mab
