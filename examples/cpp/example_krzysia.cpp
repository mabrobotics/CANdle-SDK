#include <unistd.h>
#include <iostream>
#include "candle.hpp"
// Create CANdle object and set FDCAN baudrate to 1Mbps
static mab::Candle candle(mab::CAN_BAUD_1M, true);
static bool        keepTransmitting = false;
static float       targetPos        = 0.f;
void               transfer()
{
    float temp      = 0;
    int   readCount = 10;
    while (keepTransmitting)
    {
        candle.writeMd80Register(0x64, mab::Md80Reg_E::targetPosition, targetPos);
        if (readCount == 0)
        {
            candle.readMd80Register(0x64, mab::Md80Reg_E::motorTemperature, temp);
            readCount = 10;
        }
        readCount -= 1;
    }
}
int main()
{
    std::thread transmitter;
    // Ping FDCAN bus in search of drives
    auto ids = candle.ping();
    if (ids.size() == 0)  // If no drives found -> quit
        return EXIT_FAILURE;
    // Add all found to the update list
    for (auto& id : ids)
        candle.addMd80(id);
    candle.controlMd80SetEncoderZero(ids[0]);  // Reset encoder at current position
    candle.controlMd80Mode(ids[0], mab::Md80Mode_E::POSITION_PID);  // Set mode to position PID
    candle.controlMd80Enable(ids[0], true);                         // Enable the drive
    // We will run both Position PID and Velocity PID at default settings. If you wish you can play
    // with the parameters Using the methods below:
    candle.md80s[0].setPositionControllerParams(20.0f, 0.2f, 0.0f, 15.0f);
    // candle.md80s[0].setVelocityControllerParams(0.5f, 0.1f, 0.0f, 1.5f);
    // candle.md80s[0].setProfileVelocity(5.0);
    // candle.md80s[0].setMaxTorque(0.5f);
    // To reload default controller parameters, simply disable the drive (contorlMd80Enable(id,
    // false)), stop the communications (candle.end()) or power cycle the drive (off-on).
    float t  = 0.f;
    float dt = 0.02f;
    // Begin update loop (it starts in the background)
    // candle.begin();
    // candle.beginCustom();
    // candle.transmitNewStdFrame();
    keepTransmitting = true;
    // std::cout << "start thread" << std::endl;
    transmitter = std::thread(&transfer);
    for (int i = 0; i < 1000; i++)
    {
        t += dt;
        // candle.md80s[0].setTargetPosition(sin(t) * 2.0f);
        targetPos = sin(t) * 2.f;
        // std::cout << "Drive ID = " << candle.md80s[0].getId()
        //           << " Velocity: " << candle.md80s[0].getVelocity() << std::endl;
        // candle.writeMd80Register(100, Md80Reg_E::targetPosition, )
        usleep(10000);
    }
    // std::cout << "Main loop finished" << std::endl;
    keepTransmitting = false;
    // if (transmitter.joinable())
    // std::cout << "joint thread" << std::endl;
    transmitter.join();
    // Close the update loop
    // candle.end();
    return EXIT_SUCCESS;
}