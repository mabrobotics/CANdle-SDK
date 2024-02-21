#ifndef MD80_HPP
#define MD80_HPP

#include <cstdint>
#include <map>
#include <memory>

#include "IObjectDictionaryParser.hpp"
#include "ObjectDictionaryParserEDS.hpp"

class MD80
{
   public:
	MD80()
	{
		ObjectDictionaryParserEDS parser{};
		parser.parseFile("C:/Users/klonyyy/PROJECTS/MAB/projects/MD80/code/md80_firmware/CANopenNode_STM32/MD80_DS402.eds", OD);
		// C:/Users/klonyyy/PROJECTS/MAB/projects/MD80/code/md80_firmware/CANopenNode_STM32
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