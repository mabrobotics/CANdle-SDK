#include "pds.hpp"

#include <string>

enum moduleType_E : uint8_t
{
	UNDEFINED = 0x00,
	BRAKE_RESISTOR,
	IC12,
	IC5,
	POWER_STAGE_V1,
	POWER_STAGE_V2,
	/* NEW MODULE TYPES HERE */
};

static std::string type2String(moduleType_E type)
{
	switch (type)
	{
		case moduleType_E::UNDEFINED:
			return "UNDEFINED";
		case moduleType_E::BRAKE_RESISTOR:
			return "BRAKE_RESISTOR";
		case moduleType_E::IC12:
			return "IC12";
		case moduleType_E::IC5:
			return "IC5";
		case moduleType_E::POWER_STAGE_V1:
			return "POWER_STAGE_V1";
		case moduleType_E::POWER_STAGE_V2:
			return "POWER_STAGE_V2";
		/* NEW MODULE TYPES HERE */
		default:
			return "UNKNOWN";
	}
}

enum moduleVersion_E : uint8_t
{
	UNKNOWN = 0x00,
	V0_1,  // 0.1
	V0_2,  // 0.2
	V0_3,  // 0.3
		   /* NEW MODULE VERSIONS HERE */
};

static std::string version2String(moduleVersion_E version)
{
	switch (version)
	{
		case moduleVersion_E::UNKNOWN:
			return "UNKNOWN";
		case moduleVersion_E::V0_1:
			return "V0_1";	// 0.1
		case moduleVersion_E::V0_2:
			return "V0_2";	// 0.2
		case moduleVersion_E::V0_3:
			return "V0_3";	// 0.3
		/* NEW MODULE VERSIONS HERE */
		default:
			return "UNKNOWN_VERSION";
	}
}

typedef struct __attribute__((packed)) moduleData_S
{
	bool			isDetected;
	moduleType_E	type;
	moduleVersion_E version;
	float			temperature;
	uint16_t		busVoltage;
	int16_t			current;
	bool			isEnabled;
} moduleData_S;

constexpr size_t NUMBER_OF_MODULES = 3u;

typedef struct __attribute__((packed)) pdsInfo_S
{
	moduleData_S module[NUMBER_OF_MODULES];

} pdsInfo_S;

namespace mab
{

	Pds::Pds(uint16_t canId, std::shared_ptr<Candle> sp_Candle)
		: msp_Candle(sp_Candle), m_canId(canId)
	{
		m_log.tag	= "PDS";
		m_log.level = logger::LogLevel_E::DEBUG;
	}

	void Pds::getPdsInfo(void)
	{
		const char txBuffer[]	= {FRAME_GET_INFO, 0x00};
		char	   rxBuffer[64] = {0};

		pdsInfo_S pdsInfo = {0};

		m_log.info("About to get info from PDS device with CAN ID [ %u ]", m_canId);

		if (msp_Candle->sendGenericFDCanFrame(m_canId, sizeof(txBuffer), txBuffer, rxBuffer, 100))
		{
			m_log.success("Response from PDS received!");
			memcpy(&pdsInfo, rxBuffer, sizeof(pdsInfo_S));

			m_log.info("PDS modules:");

			for (uint8_t nModule = 0; nModule < NUMBER_OF_MODULES; nModule++)
			{
				if (pdsInfo.module[nModule].isDetected)
				{
					m_log.info("MODULE [ %u ] DETECTED", nModule);
					m_log.info("\tType : [ %s ]",
							   type2String(pdsInfo.module[nModule].type).c_str());
					m_log.info("\tVersion : [ %s ]",
							   version2String(pdsInfo.module[nModule].version).c_str());
					m_log.info("\tEnabled [ %s ]",
							   pdsInfo.module[nModule].isEnabled ? "YES" : "NO");
					m_log.info("\tBus voltage : [ %u ]", pdsInfo.module[nModule].busVoltage);
					m_log.info("\tCurrent : [ %d ]", pdsInfo.module[nModule].current);
					m_log.info("\tTemperature : [ %.2f ]", pdsInfo.module[nModule].temperature);
				}
				else
				{
					m_log.info("MODULE [ %u ] NOT DETECTED", nModule);
				}
			}
		}
		else
		{
			m_log.error("PDS not responding");
		}
	}

}  // namespace mab