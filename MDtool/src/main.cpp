#include <span>
#include "Candle.hpp"
#include "CandleInterface.hpp"
#include "UsbHandler.hpp"
#include "CLI11.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "Downloader.hpp"
#include <iostream>

class Mdtool
{
public:
    ~Mdtool()
    {
        candle->deInit();
    }

    bool init(ICommunication *interface, spdlog::logger *logger, Candle::Baud baud)
    {
        logger->info("Initalizing...");
        this->logger = logger;
        this->interface = interface;
        candle = std::make_unique<Candle>(interface);
        return candle->init(baud);
    }

    void ping()
    {
        auto drives = candle->ping();
        logger->info("Found drives: ");

        for (auto &md80 : drives)
            logger->info(std::to_string(md80));
    }

    void updateMd80()
    {
        candle->deInit();
        Downloader downloader(interface, logger);
    }

private:
    std::unique_ptr<Candle> candle;
    ICommunication *interface;
    spdlog::logger *logger;
};

int main(int argc, char **argv)
{
    CLI::App app{"MDtool"};
    auto *ping = app.add_subcommand("ping", "Discovers all drives connected to CANdle");
    auto *updateMD80 = app.add_subcommand("update_md80", "Use to update MD80");
    auto *updateCANdle = app.add_subcommand("update_candle", "Use to update CANdle");

    auto logger = spdlog::stdout_color_mt("console");
    logger->set_pattern("[%^%l%$] %v");

    uint32_t baud = 1;
    ping->add_option("-b,--baud", baud, "CAN Baudrate in Mbps used to setup CANdle");

    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "Use for verbose mode");

    CLI11_PARSE(app, argc, argv);

    Mdtool mdtool;
    std::unique_ptr<IBusHandler> busHandler = std::make_unique<UsbHandler>();
    std::unique_ptr<ICommunication> candleInterface = std::make_unique<CandleInterface>(busHandler.get());

    mdtool.init(candleInterface.get(), logger.get(), static_cast<Candle::Baud>(baud));

    if (app.got_subcommand("ping"))
        mdtool.ping();

    else if (app.got_subcommand("update_md80"))
        mdtool.updateMd80();

    return 0;
}
