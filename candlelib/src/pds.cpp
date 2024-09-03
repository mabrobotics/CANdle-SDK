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
	uint16_t		temperature;
	uint16_t		busVoltage;
	bool			isEnabled;

} moduleData_S;

typedef struct __attribute__((packed)) pdsInfo_S
{
	moduleData_S module1;
	moduleData_S module2;
	moduleData_S module3;
	moduleData_S module4;
	moduleData_S module5;
	moduleData_S module6;

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

			if (pdsInfo.module1.isDetected)
			{
				m_log.info("MODULE [ 1 ] DETECTED");
				m_log.info("\tType : [ %s ]", type2String(pdsInfo.module1.type).c_str());
				m_log.info("\tVersion : [ %s ]", version2String(pdsInfo.module1.version).c_str());
				m_log.info("\tBus voltage : [ %u ]", pdsInfo.module1.busVoltage);
			}
			else
			{
				m_log.info("MODULE [ 1 ] NOT DETECTED");
			}

			if (pdsInfo.module2.isDetected)
			{
				m_log.info("MODULE [ 2 ] DETECTED");
				m_log.info("\tType : [ %s ]", type2String(pdsInfo.module2.type).c_str());
				m_log.info("\tVersion : [ %s ]", version2String(pdsInfo.module2.version).c_str());
				m_log.info("\tBus voltage : [ %u ]", pdsInfo.module2.busVoltage);
			}
			else
			{
				m_log.info("MODULE [ 2 ] NOT DETECTED");
			}

			if (pdsInfo.module3.isDetected)
			{
				m_log.info("MODULE [ 3 ] DETECTED");
				m_log.info("\tType : [ %s ]", type2String(pdsInfo.module3.type).c_str());
				m_log.info("\tVersion : [ %s ]", version2String(pdsInfo.module3.version).c_str());
				m_log.info("\tBus voltage : [ %u ]", pdsInfo.module3.busVoltage);
			}
			else
			{
				m_log.info("MODULE [ 3 ] NOT DETECTED");
			}

			if (pdsInfo.module4.isDetected)
			{
				m_log.info("MODULE [ 4 ] DETECTED");
				m_log.info("\tType : [ %s ]", type2String(pdsInfo.module4.type).c_str());
				m_log.info("\tVersion : [ %s ]", version2String(pdsInfo.module4.version).c_str());
				m_log.info("\tBus voltage : [ %u ]", pdsInfo.module4.busVoltage);
			}
			else
			{
				m_log.info("MODULE [ 4 ] NOT DETECTED");
			}

			if (pdsInfo.module5.isDetected)
			{
				m_log.info("MODULE [ 5 ] DETECTED");
				m_log.info("\tType : [ %s ]", type2String(pdsInfo.module5.type).c_str());
				m_log.info("\tVersion : [ %s ]", version2String(pdsInfo.module5.version).c_str());
				m_log.info("\tBus voltage : [ %u ]", pdsInfo.module5.busVoltage);
			}
			else
			{
				m_log.info("MODULE [ 5 ] NOT DETECTED");
			}

			if (pdsInfo.module6.isDetected)
			{
				m_log.info("MODULE [ 6 ] DETECTED");
				m_log.info("\tType : [ %s ]", type2String(pdsInfo.module6.type).c_str());
				m_log.info("\tVersion : [ %s ]", version2String(pdsInfo.module6.version).c_str());
				m_log.info("\tBus voltage : [ %u ]", pdsInfo.module6.busVoltage);
			}
			else
			{
				m_log.info("MODULE [ 6 ] NOT DETECTED");
			}
		}
		else
		{
			m_log.error("PDS not responding");
		}
	}

}  // namespace mab