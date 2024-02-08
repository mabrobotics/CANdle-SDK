#ifndef MD80_HPP
#define MD80_HPP

#include <IObjectDictionaryParser.hpp>
#include <ObjectDictionaryParserEDS.hpp>
#include <cstdint>
#include <map>
#include <memory>

class MD80
{
   public:
	MD80()
	{
		ObjectDictionaryParserEDS parser{};
		parser.parseFile("/home/klonyyy/MAB/CANdle-SDK/CANdle_lib/src/MD80/MD80_DS402.eds", OD);
	}

	float getOutputPosition() const
	{
		return std::get<float>(OD.at(0x2009)->subEntries.at(0x01)->value);
	}

	float getOutputVelocity() const
	{
		return std::get<float>(OD.at(0x2009)->subEntries.at(0x02)->value);
	}

	float getOutputTorque() const
	{
		return std::get<float>(OD.at(0x2009)->subEntries.at(0x03)->value);
	}

	void setPositionTarget(float position)
	{
		OD.at(0x2008)->subEntries.at(0x09)->value = position;
	}

	void setVelocityTarget(float velocity)
	{
		OD.at(0x2008)->subEntries.at(0x0A)->value = velocity;
	}

	void setTorqueTarget(float torque)
	{
		OD.at(0x2008)->subEntries.at(0x0B)->value = torque;
	}

	IODParser::ODType OD;
};

#endif