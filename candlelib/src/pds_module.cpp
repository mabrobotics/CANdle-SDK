#include "pds_module.hpp"

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
