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
		parser.parseFile("C:/Users/klonyyy/PROJECTS/MAB/projects/MD80/code/md80_firmware/CANopenNode_STM32/MD80_DS402.eds", OD);
	}

	std::map<uint32_t, std::shared_ptr<IODParser::Entry>> OD;
};

#endif