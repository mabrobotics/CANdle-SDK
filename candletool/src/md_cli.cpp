#include "md_cli.hpp"
#include <string_view>
#include "candle.hpp"
#include "logger.hpp"
#include "mab_types.hpp"
#include "md_types.hpp"
#include "candle_types.hpp"

namespace mab
{
    MDCli::MDCli(CLI::App* rootCli)
    {
        auto* mdCLi = rootCli->add_subcommand("md", "MD commands");

        // Common options
        canId_t mdCanId;

        // Blink
        auto* blink = mdCLi->add_subcommand("blink", "Blink LEDs on MD drive.");
        blink->add_option("<CAN_ID>", mdCanId, "CAN ID of the MD to interact with.")->required();
        blink->callback(
            [&]()
            {
                auto md =
                    getMd(mdCanId, CANdleBaudrate_E::CAN_BAUD_1M, candleTypes::busTypes_t::USB);
                md->blink();
            });
    }

    // TODO: add path and id to flags
    std::unique_ptr<MD, std::function<void(MD*)>> MDCli::getMd(
        const canId_t                 mdCanId,
        const CANdleBaudrate_E        datarate,
        const candleTypes::busTypes_t busType)
    {
        auto                     candle  = attachCandle(datarate, busType);
        std::function<void(MD*)> deleter = [candle](MD* ptr)
        {
            delete ptr;
            detachCandle(candle);
        };
        auto md = std::unique_ptr<MD, std::function<void(MD*)>>(new MD(mdCanId, candle), deleter);
        if (md->init() == MD::Error_t::OK)
            return md;
        else
        {
            m_logger.error("Error while initializing MD with id %d", mdCanId);
            return nullptr;
        }
    }
}  // namespace mab
