#ifndef _MDTOOL_HPP
#define _MDTOOL_HPP

#include <optional>

#include "Candle.hpp"
#include "Downloader.hpp"
#include "UsbHandler.hpp"

class Mdtool
{
   public:
	~Mdtool();
	bool init(ICommunication* interface, spdlog::logger* logger, Candle::Baud baud);
	void ping();
	bool updateMd80(std::string& filePath, uint32_t id, bool recover, bool all);
	bool updateBootloader(std::string& filePath, uint32_t id, bool recover);
	bool readSDO(uint32_t id, uint16_t index, uint8_t subindex);
	bool writeSDO(uint32_t id, uint16_t index, uint8_t subindex, const IODParser::ValueType& value);

   private:
	static constexpr uint32_t secondaryBootloaderAddress = 0x8005000;
	static constexpr uint32_t primaryBootloaderAddress = 0x8000000;
	std::unique_ptr<Candle> candle;
	ICommunication* interface;
	spdlog::logger* logger;

	Candle::Baud baudrate = Candle::Baud::BAUD_1M;

   private:
	IODParser::ValueType getTypeBasedOnTag(IODParser::DataType tag);
	std::optional<IODParser::Entry*> checkEntryExists(MD80* md80, uint16_t index, uint8_t subindex);
};

#endif