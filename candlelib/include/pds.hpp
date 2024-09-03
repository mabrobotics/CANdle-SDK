#pragma once

#include "candle.hpp"
#include "logger.hpp"

namespace mab
{

	/**
	 * @brief Power distribution system class
	 *
	 */
	class Pds
	{
	  public:
		Pds() = delete;

		/**
		 * @brief Construct a new Pds object
		 *
		 * @param sp_Candle shared pointer to the Candle Object
		 * @param canId CANBus node ID of the PDS instance being created
		 * @note Note that default constructor is deleted so PDS Class is forced to take Candle
		 * dependency during creation
		 */
		Pds(uint16_t canId, std::shared_ptr<Candle> sp_Candle);

		void getPdsInfo(void);

	  private:
		/**
		 * @brief Member pointer to Candle object representing Candle device the PDS is
		 * connected to over CANBus is connected over the CANBus
		 *
		 */
		std::shared_ptr<Candle> msp_Candle = nullptr;

		logger m_log;

		uint16_t m_canId = 0;
	};

}  // namespace mab