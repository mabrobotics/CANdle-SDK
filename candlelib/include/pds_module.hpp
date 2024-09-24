#pragma once

namespace mab
{

	/**
	 * @brief Power distribution system class
	 *
	 */
	class PdsModule
	{
	  public:
		enum class type_E : uint8_t
		{
			// Undefined means that module is not connected or PDS could not determine its type.
			UNDEFINED = 0x00,
			BRAKE_RESISTOR,
			ISOLATED_CONVERTER_12V,
			ISOLATED_CONVERTER_5V,
			POWER_STAGE_V1,
			POWER_STAGE_V2,
			/* NEW MODULE TYPES HERE */
		};

		enum class socketIndex_E : uint8_t
		{
			SOCKET_1 = 0x00,
			SOCKET_2,
			SOCKET_3,
			SOCKET_4,
			SOCKET_5,
			SOCKET_6,

		};

		PdsModule() = delete;
		PdsModule(socketIndex_E socketIndex);

		// static std::string moduleType2String(moduleType_E type);

	  private:
		logger m_log;

		// Represents physical socket index number that the particular module is connected to.
		socketIndex_E m_socketIndex;
	};

}  // namespace mab