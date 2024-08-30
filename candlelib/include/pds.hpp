#pragma once

#include "candle.hpp"

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
		 * @note Note that default constructor is deleted so PDS Class is forced to take Candle
		 * dependency during creation
		 */
		Pds(std::shared_ptr<Candle> sp_Candle);

	  private:
		/**
		 * @brief Member pointer to Candle object representing Candle device the PDS is connected to
		 * over CANBus is connected over the CANBus
		 *
		 */
		std::shared_ptr<Candle> msp_Candle = nullptr;
	};

}  // namespace mab