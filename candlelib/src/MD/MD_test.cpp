#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <functional>

#include "MD.hpp"
#include "md_types.hpp"

class MD_test : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }
};

TEST_F(MD_test, serializeDeserializeRegisters)
{
    // auto registerStruct               = mab::MDRegisters_S();
    // registerStruct.canBaudrate        = 1'000'000;
    // registerStruct.motorName.value[0] = 'a';
    // registerStruct.motorName.value[1] = 'b';
    // registerStruct.motorName.value[2] = 'c';
    // registerStruct.motorName.value[3] = 'd';
    // registerStruct.dcBusVoltage       = 24000;

    // auto registersOut = std::make_tuple(std::reference_wrapper(registerStruct.canBaudrate),
    //                                     std::reference_wrapper(registerStruct.motorName),
    //                                     std::reference_wrapper(registerStruct.dcBusVoltage));

    // auto serialized = mab::MD::serializeMDRegisters(registersOut);

    // ASSERT_EQ(serialized.size(),
    //           (3 * sizeof(u16) + registerStruct.canBaudrate.getSize() +
    //            registerStruct.motorName.getSize() + registerStruct.dcBusVoltage.getSize()));

    // registerStruct.canBaudrate        = 5'000'000;
    // registerStruct.motorName.value[0] = 'c';
    // registerStruct.motorName.value[1] = 'c';
    // registerStruct.motorName.value[2] = 'd';
    // registerStruct.motorName.value[3] = 'a';
    // registerStruct.dcBusVoltage       = 48000;
    // auto registersIn = std::make_tuple(std::reference_wrapper(registerStruct.canBaudrate),
    //                                    std::reference_wrapper(registerStruct.motorName),
    //                                    std::reference_wrapper(registerStruct.dcBusVoltage));

    // mab::MD::deserializeMDRegisters(serialized, registersIn);
    // ASSERT_EQ(std::get<0>(registersOut).value, std::get<0>(registersIn).value);
    // ASSERT_EQ(std::get<2>(registersOut).value, std::get<2>(registersIn).value);
}