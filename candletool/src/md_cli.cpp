#include "md_cli.hpp"
#include <stdexcept>
#include <string_view>
#include "candle.hpp"
#include "logger.hpp"
#include "mab_types.hpp"
#include "md_types.hpp"
#include "candle_types.hpp"

namespace mab
{
    MDCli::MDCli(CLI::App* rootCli, const std::shared_ptr<const CandleBuilder> candleBuilder)
    {
        if (candleBuilder == nullptr)
        {
            throw std::runtime_error("MDCli arguments can not be nullptr!");
        }
        auto*                          mdCLi   = rootCli->add_subcommand("md", "MD commands");
        const std::shared_ptr<canId_t> mdCanId = std::make_shared<canId_t>(100);
        mdCLi->add_option("<CAN_ID>", *mdCanId, "CAN ID of the MD to interact with.")->required();

        // Blink
        auto* blink = mdCLi->add_subcommand("blink", "Blink LEDs on MD drive.");
        blink->callback(
            [this, candleBuilder, mdCanId]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                md->blink();
            });

        // // Can
        // auto* can = mdCLi->add_subcommand(
        //     "can", "Configure CAN network parameters id, datarate and timeout.");
        // std::string configPathTo can->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->can();
        //         logger::info("CAN command placeholder");
        //     });

        // // Calibration
        // auto* calibration = mdCLi->add_subcommand("calibration", "Calibrate the MD drive.");
        // calibration->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->calibrate();
        //         logger::info("Calibration command placeholder");
        //     });

        // // Clear
        // auto* clear = mdCLi->add_subcommand("clear", "Clear MD drive errors and warnings.");
        // clear->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->clear();
        //         logger::info("Clear command placeholder");
        //     });

        // // Config
        // auto* config = mdCLi->add_subcommand("config", "Configure MD drive.");
        // config->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->config();
        //         logger::info("Config command placeholder");
        //     });

        // // Discover
        // auto* discover = mdCLi->add_subcommand("discover", "Discover MD drives on the network.");
        // discover->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->discover();
        //         logger::info("Discover command placeholder");
        //     });

        // // Info
        // auto* info = mdCLi->add_subcommand("info", "Get information about the MD drive.");
        // info->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->info();
        //         logger::info("Info command placeholder");
        //     });

        // // Register
        // auto* reg = mdCLi->add_subcommand("register", "Register operations for MD drive.");
        // reg->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->register();
        //         logger::info("Register command placeholder");
        //     });

        // // Test
        // auto* test = mdCLi->add_subcommand("test", "Test the MD drive.");
        // test->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->test();
        //         logger::info("Test command placeholder");
        //     });

        // // Update
        // auto* update = mdCLi->add_subcommand("update", "Update firmware on MD drive.");
        // update->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->update();
        //         logger::info("Update command placeholder");
        //     });
    }

    std::unique_ptr<MD, std::function<void(MD*)>> MDCli::getMd(
        const std::shared_ptr<canId_t>             mdCanId,
        const std::shared_ptr<const CandleBuilder> candleBuilder)
    {
        auto candle = candleBuilder->build().value_or(nullptr);
        if (candle == nullptr)
        {
            return (nullptr);
        }
        std::function<void(MD*)> deleter = [candle](MD* ptr)
        {
            delete ptr;
            detachCandle(candle);
        };
        auto md = std::unique_ptr<MD, std::function<void(MD*)>>(new MD(*mdCanId, candle), deleter);
        if (md->init() == MD::Error_t::OK)
            return md;
        else
        {
            return nullptr;
        }
    }
}  // namespace mab
