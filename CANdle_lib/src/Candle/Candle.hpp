/**
 * @file Candle.hpp
 * @author Piotr Wasilewski (piotr.wasilewski@mabrobotics.pl)
 * @brief
 * @version 0.1
 * @date 2024-03-07
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef CANDLE_HPP
#define CANDLE_HPP

#include <iostream>
#include <memory>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "Barrier.hpp"
#include "BusHandler/UsbHandler.hpp"
#include "CanopenStack/CANopen/CanopenStack.hpp"
#include "Communication/CandleInterface.hpp"
#include "ICommunication.hpp"
#include "MD80.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

/**
 * @brief Main Candle class that is created by the user to control MD80 drives.
 *
 */
class Candle
{
   public:
	/**
	 * @brief An enum holding possible baudrate settings.
	 *
	 * These are single-value baudrate settings. The value is interpreted on the CANdle side to prescalers and bit quantas. It is possible to add custom settings in the future with the help of a more sophisticated can baudrate frame.
	 */
	enum class Baud : uint32_t
	{
		BAUD_1M = 1,
		BAUD_8M = 8,
	};

	/**
	 * @brief An enum holding possible modes of operations of the MD80 controller. Some fields are CiA402 - compliant.
	 *
	 */
	enum class ModesOfOperation : int8_t
	{
		IMPEDANCE = -3,
		SERVICE = -2,
		IDLE = 0,
		PROFILE_POSITION = 1,
		PROFILE_VELOCITY = 2,
		CYCLIC_SYNC_POSITION = 8,
		CYCLIC_SYNC_VELOCTIY = 9,
	};

	/**
	 * @brief Construct a new Candle object.
	 *
	 * Construct Candle object without explicitly providing the logger and communication interface.
	 */
	Candle();

	/**
	 * @brief Construct a new Candle object.
	 *
	 * Construct Candle object explicitly passing communication interface and logger instances.
	 * @param interface ICommunication interface derived class object.
	 * @param logger spdlog object.
	 */
	Candle(std::shared_ptr<ICommunication> interface, std::shared_ptr<spdlog::logger> logger);

	Candle(const Candle&) = delete;
	Candle& operator=(const Candle&) = delete;

	/**
	 * @brief Destroy the Candle object.
	 *
	 * Ends the receive and transmit threads.
	 */
	~Candle();

	/**
	 * @brief Initialize function that should be called before any operation on Candle class.
	 *
	 * Initializes the communication interface and launches receive and transmit threads.
	 * @param baud baudrate setting of the MD80 controllers.
	 * @param edsPath path to EDS file, named "MD80_DS402.eds". If left empty the default search path is the path from which the script is running.
	 * @return true
	 * @return false
	 */
	bool init(Baud baud = Baud::BAUD_1M, std::string edsPath = "");
	/**
	 * @brief Deinits Candle object
	 *
	 * Receive and Transmit threads are ended manually.
	 */
	void deInit();
	/**
	 * @brief Set the state and duration between consecutive SYNC messages that trigger RXPDO responses from MD80
	 *
	 * @param state true to turn on, false to turn off.
	 * @param intervalUs interval between frames in microseconds.
	 */
	void setSendSync(bool state, uint32_t intervalUs);
	/**
	 * @brief Discover connected MD80 controllers' IDs on all CANdle channels.
	 *
	 * This function does not care on which channels the MD80 was discovered.
	 * @return std::vector<uint32_t>
	 */
	std::vector<uint32_t> ping();
	/**
	 * @brief Discover connected MD80 controllers' IDs and channels
	 *
	 * This function returns a vector of std::pairs of ID and the channel it was discovered on
	 * @return std::vector<std::pair<uint32_t, uint8_t>>
	 */
	std::vector<std::pair<uint32_t, uint8_t>> pingWithChannel();
	/**
	 * @brief Get the channel the MD80 is on based on its ID.
	 *
	 * @param id id of the MD80
	 * @return uint8_t channel number
	 */
	uint8_t getChannelBasedOnId(uint32_t id);
	/**
	 * @brief Adds MD80 to an internal Candle MD80 list.
	 *
	 * This function created MD80 object internally. The MD80 object can be retrieved using the \ref getMd80() function.
	 * @param id
	 * @return true
	 * @return false
	 */
	bool addMd80(uint32_t id);
	/**
	 * @brief Get the Md80 object
	 *
	 * Returns a std::shared_ptr to the MD80 object.
	 * @param id
	 * @return std::shared_ptr<MD80>
	 */
	std::shared_ptr<MD80> getMd80(uint32_t id) const;
	/**
	 * @brief Enter operational mode on MD80. Movement is allowed only in operational mode.
	 *
	 * @param id
	 * @return true
	 * @return false
	 */
	bool enterOperational(uint32_t id);
	/**
	 * @brief Enter switch on disabled state. Movement is not allowed in this mode.
	 *
	 * @param id
	 * @return true
	 * @return false
	 */
	bool enterSwitchOnDisabled(uint32_t id);
	/**
	 * @brief Set mode of operation.
	 *
	 * @param id
	 * @param mode \ref ModesOfOperation
	 * @return true
	 * @return false
	 */
	bool setModeOfOperation(uint32_t id, ModesOfOperation mode);
	/**
	 * @brief Set zero position on selected MD80.
	 *
	 * @param id
	 * @return true
	 * @return false
	 */
	bool setZeroPosition(uint32_t id);
	/**
	 * @brief Reset MD80 controller.
	 *
	 * @param id
	 * @return true
	 * @return false
	 */
	bool reset(uint32_t id);

	/**
	 * @brief Setup PDO objects
	 *
	 * This function can be used to setup the response and command frames structures. Always check if the total field length does not exceed the max frame length (1M baudrate max is 8 bytes, 8M baudrate is 64 bytes). RPDOs are sent before SYNC msg, TPDO are received after SYNC msg. SYNC must be turned on using \ref setSendSync().
	 * @param id
	 * @param pdoID ID of the RPDO (response) or TPDO (command) \ref CanopenStack::PDO
	 * @param fields std::vector of std::pairs of index and subindex from the OD that are expected in the response or command
	 * @return true
	 * @return false
	 */
	bool setupPDO(uint32_t id, CanopenStack::PDO pdoID, const std::vector<std::pair<uint16_t, uint8_t>>& fields);

	/**
	 * @brief Used to write a specific field in OD, given the index, subindex, and value
	 *
	 * @tparam T type of the field, one of \ref ValueType
	 * @param id
	 * @param index_ OD index.
	 * @param subindex_ OD subindex.
	 * @param value value to be written.
	 * @return true
	 * @return false
	 */
	template <typename T>
	bool writeSDO(uint32_t id, uint16_t index_, uint8_t subindex_, const T& value)
	{
		uint32_t errorCode = 0;
		bool result = canopenStack->writeSDO(id, index_, subindex_, value, errorCode);

		if (errorCode)
		{
			logger->error("SDO write error (0x{:x}:0x{:x})! Code {:x}", index_, subindex_, errorCode);
			return false;
		}

		return result;
	}

	/**
	 * @brief Used to read a specific field in OD, given the index, subindex, and value
	 *
	 * @tparam T type of the field, one of \ref ValueType
	 * @param id
	 * @param index_ OD index.
	 * @param subindex_ OD subindex.
	 * @param value value reference to which the read value will be written.
	 * @param checkOD set to false if checking the type against the OD is not needed and the value should not be written to the MD80s OD, only returned using the value reference.
	 * @return true
	 * @return false
	 */
	template <typename T>
	bool readSDO(uint32_t id, uint16_t index_, uint8_t subindex_, T& value, bool checkOD = true)
	{
		uint32_t errorCode = 0;
		bool result = canopenStack->readSDO(id, index_, subindex_, value, errorCode, checkOD);

		if (errorCode)
		{
			logger->error("SDO read error (0x{:x}:0x{:x})! Error code: 0x{:x}", index_, subindex_, errorCode);
			return false;
		}

		return result;
	}

   private:
	/**
	 * @brief Receive thread
	 *
	 */
	void receiveHandler();
	/**
	 * @brief Transmit thread
	 *
	 */
	void transmitHandler();
	/**
	 * @brief Candle device status handler. Periodically checks internal Candle state and issues warnings if FIFO max fill levels exceed thresholds.
	 *
	 */
	void handleCandleDeviceStatus();

   public:
	std::unique_ptr<CanopenStack> canopenStack;

   private:
	static constexpr uint8_t rxFifoWarningLevel = 50;
	static constexpr uint8_t txFifoWarningLevel = 50;
	static constexpr uint8_t rxFifoErrorLevel = 99;
	static constexpr uint8_t txFifoErrorLevel = 99;
	inline static size_t candleNum;

	uint32_t candleChannels = 1;

	std::thread receiveThread;
	std::thread transmitThread;
	Barrier syncPoint;

	std::atomic<bool> done = false;
	std::atomic<bool> sendSync = false;

	std::atomic<uint32_t> syncIntervalUs = 10000;

	bool isInitialized = false;

	std::unordered_map<uint32_t, std::shared_ptr<MD80>> md80s;
	std::shared_ptr<ICommunication> interface;
	std::shared_ptr<spdlog::logger> logger;

	std::string edsPath;
};

#endif