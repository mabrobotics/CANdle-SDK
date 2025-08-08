#pragma once

#include <string>
#include "candle.hpp"
#include "mini/ini.h"
#include "logger.hpp"
#include "pds.hpp"
#include "MDCO.hpp"
#include "edsParser.hpp"

struct UserCommandCO
{
    std::string variant     = "";
    u32         id          = 0;
    u32         newId       = 0;
    std::string baud        = "1M";
    u32         canWatchdog = 100;
    f32         current     = 1.f;
    f32         bandwidth   = 100.f;
    std::string cfgPath     = "";
    f32         pos = 0.f, vel = 10.f, acc = 5.f, dcc = 5.f;
    bool        infoAll          = false;
    std::string reg              = "0x0000";
    std::string value            = "";
    bool        force            = false;
    std::string firmwareFileName = "";
    bool        noReset          = false;
    u16         maxCurrent       = 500;
    u16         maxTorque        = 200;
    u32         ratedCurrent     = 1000;
    u32         ratedTorque      = 1000;
    u32         MaxSpeed         = 1000;
    i32         desiredSpeed     = 50;
    u8          subReg           = 0;
    u8          dataSize         = 0;
    u32         HeartbeatTimeout = 1000;
    f32         kp               = 1;
    f32         kd               = 0;
    u16         torque           = 0;
    edsObject   edsObj;  // used for EDS parser
};

/// @brief return the number of millis since 1st january 1970
/// @return the number of millis since 1st january 1970
uint64_t get_time_us();

class CandleToolCO
{
  public:
    /// @brief Constructor for CandleToolCO
    /// @param baud The baud rate for the CANdle device
    explicit CandleToolCO(mab::CANdleBaudrate_E baud);

    /// @brief Destructor for CandleToolCO
    ~CandleToolCO();

    /// @brief Initialize the CandleToolCO with a specific CANdle device
    /// @param variant Not implemented yet
    void ping(const std::string& variant);

    /// @brief modifies the CANdle device configuration
    /// @param id The CAN ID of the device to configure
    /// @param newId The new CAN ID to set
    /// @param baud The baud rate to set
    /// @param timeout The timeout for the CAN communication
    /// @param termination Optional parameter
    void configCan(u16 id, u16 newId, const std::string& baud, u16 timeout, bool termination = 0);

    /// @brief Save the current configuration of the device
    /// @param id The CAN ID of the device to save the configuration
    void configSave(u16 id);

    /// @brief Set the zero position of the device
    /// @param id The CAN ID of the device to set the zero position
    void configZero(u16 id);

    /// @brief Set the maximum current for the device
    /// @param id The CAN ID of the device to configure
    /// @param current The maximum current in Amps to set
    void configCurrent(u16 id, f32 current);

    /// @brief Set the torque bandwidth of the device
    /// @param id The CAN ID of the device to configure
    /// @param bandwidth The desired torque bandwidth in Hz
    /// @note The bandwidth must be between 50 and 2500 Hz
    void configBandwidth(u16 id, f32 bandwidth);

    /// @brief Move the device to a target speed with pdo message
    /// @param id The CAN ID of the device to move
    /// @param desiredSpeed The desired speed in RPM to set
    void sendPdoSpeed(u16 id, i32 desiredSpeed);

    /// @brief Move the device to a target position with pdo message
    /// @param id The CAN ID of the device to move
    /// @param desiredSpeed The desired speed in RPM to set
    /// @note This function is used to send a PDO message to the device to move it
    void sendPdoPosition(u16 id, i32 desiredSpeed);

    /// @brief Send a custom pdo to the MD
    /// @param id ID of the motor to send the messsage
    /// @param Desiregister the desire PDO register (0x1600/0x1601/0x1602/0x1603)
    /// @param data data to send (little endian)
    void SendCustomPdo(u16 id, const edsObject& Desiregister, u64 data);

    /// @brief Run a calibration
    /// @param id The CAN ID of the device to calibrate
    void setupCalibration(u16 id);

    /// @brief Setup the calibration on the output encoder
    /// @param id The CAN ID of the device to setup
    void setupCalibrationOutput(u16 id);

    /// @brief Setup the md with a specific configuration file
    /// @param id The CAN ID of the device to setup
    void setupMotor(u16 id, const std::string& cfgPath, bool force);

    /// @brief Display the info of the md
    /// @param id The CAN ID of the device to display info
    /// @param printAll If true, print all info, otherwise print only the basic info
    void setupInfo(u16 id, bool printAll);

    /// @brief Read a specific configuration from the md
    /// @param id The CAN ID of the device to read the configuration from
    /// @param cfgName The name of the configuration to read
    void setupReadConfig(u16 id, const std::string& cfgName);

    /// @brief reach a target position as fast as possible
    /// @param id The CAN ID of the device to move
    /// @param targetPosition The target position in encoder increments to reach
    /// @param MaxSpeed The maximum speed in RPM to reach the target position
    /// @param MaxCurrent The maximum current in mA to use
    /// @param RatedCurrent The rated current in mA of the motor
    /// @param MaxTorque The maximum torque in mNM to use
    /// @param RatedTorque The rated torque in mNM of the motor
    void testMove(u16 id,
                  f32 targetPosition,
                  u32 MaxSpeed     = 200,
                  u16 MaxCurrent   = 500,
                  u32 RatedCurrent = 1000,
                  u16 MaxTorque    = 200,
                  u32 RatedTorque  = 1000);

    /// @brief reach a target position with acceleration and deceleration limits
    /// @param id The CAN ID of the device to move
    /// @param targetPos The target position in encoder increments to reach
    /// @param accLimit The acceleration limit in RPM/s
    /// @param dccLimit The deceleration limit in RPM/s
    /// @param velLimit The maximum speed in RPM to reach the target position
    /// @param MaxCurrent The maximum current in mA to use
    /// @param RatedCurrent The rated current in mA of the motor
    /// @param MaxTorque The maximum torque in mNM to use
    /// @param RatedTorque The rated torque in mNM of the motor
    void testMoveAbsolute(u16 id,
                          f32 targetPos,
                          f32 velLimit,
                          f32 accLimit,
                          f32 dccLimit,
                          u16 MaxCurrent,
                          u32 RatedCurrent,
                          u16 MaxTorque,
                          u32 RatedTorque);

    /// @brief Move the device to a target speed
    /// @param id The CAN ID of the device to move
    /// @param MaxCurrent The maximum current in mA to use
    /// @param RatedCurrent The rated current in mA of the motor
    /// @param MaxTorque The maximum torque in mNM to use
    /// @param RatedTorque The rated torque in mNM of the motor
    /// @param MaxSpeed The maximum speed in RPM to reach the target speed
    /// @param DesiredSpeed The desired speed in RPM to set
    void testMoveSpeed(u16 id,
                       u16 MaxCurrent,
                       u32 RatedCurrent,
                       u16 MaxTorque,
                       u32 RatedTorque,
                       u32 MaxSpeed,
                       i32 DesiredSpeed);

    /// @brief Move the device to a target position with impedance control
    /// @param id The CAN ID of the device to move
    /// @param desiredSpeed The desired speed in RPM to set
    /// @param targetPos The target position in encoder increments to reach
    /// @param kp The proportional gain to use
    /// @param kd The derivative gain to use
    /// @param torque The torque to apply in mNM
    /// @param MaxSpeed The maximum speed in RPM to reach the target position
    /// @param MaxCurrent The maximum current in mA to use
    /// @param RatedCurrent The rated current in mA of the motor
    /// @param MaxTorque The maximum torque in mNM to use
    /// @param RatedTorque The rated torque in mNM of the motor
    void testMoveImpedance(u16 id,
                           i32 desiredSpeed,
                           f32 targetPos,
                           f32 kp,
                           f32 kd,
                           i16 torque,
                           u32 MaxSpeed,
                           u16 MaxCurrent,
                           u32 RatedCurrent,
                           u16 MaxTorque,
                           u32 RatedTorque);

    /// @brief Test the latency of the device
    /// @param id The CAN ID of the device to test
    /// @note This function measures the time it takes to send a command and receive a response
    void testLatency(u16 id);

    /// @brief Test the encoder output of the device
    /// @param id The CAN ID of the device to test
    void testEncoderOutput(u16 id);

    /// @brief Test the encoder main of the device
    /// @param id The CAN ID of the device to test
    void testEncoderMain(u16 id);

    /// @brief Blink the built-in LEDs of the device
    /// @param id The CAN ID of the device to blink the LEDs
    void blink(u16 id);

    /// @brief Read the encoder value of the device
    /// @param id The CAN ID of the device to read the encoder value
    void encoder(u16 id);

    /// @brief restart and clear the status of the device
    /// @param id The CAN ID of the device to read the status
    void clearErrors(u16 id, const std::string& level);

    /// @brief Reset the device
    /// @param id The CAN ID of the device to reset
    void reset(u16 id);

    /// @brief Write a value to a specific register of the device
    /// @param id The CAN ID of the device to write the value
    /// @param reg The register to write the value to
    /// @param value The value to write to the register
    /// @param subIndex The sub-index of the register to write to
    /// @param dataSize The size of the data to write
    void registerWrite(
        u16 id, u16 reg, const std::string& value, u8 subIndex, u8 dataSize, bool force);

    /// @brief Read a value from a specific register of the device
    /// @param id The CAN ID of the device to read the value
    /// @param reg The register to read the value from
    /// @param subIndex The sub-index of the register to read from
    void SDOsegmentedRead(u16 id, u16 reg, u8 subIndex);

    /// @brief Write a value to a specific register of the device using segmented SDO
    /// @param id The CAN ID of the device to write the value
    /// @param reg The register to write the value to
    /// @param subIndex The sub-index of the register to write to
    /// @param data The data to write to the register
    void SDOsegmentedWrite(u16 id, u16 reg, u8 subIndex, std::string& data);

    /// @brief Read a value from a specific register of the device using expedited SDO
    /// @param id The CAN ID of the device to read the value
    /// @param reg The register to read the value from
    /// @param subIndex The sub-index of the register to read from
    void registerRead(u16 id, u16 reg, u8 subIndex, bool force);

    /// @brief clean a string entry
    /// @param s the string we need to clean
    /// @return the cleaned string
    static std::string clean(std::string s);

    /// @brief Perform a heartbeat test between two devices
    /// @param MasterId The CAN ID of the master device
    /// @param SlaveId The CAN ID of the slave device
    /// @param HeartbeatTimeout The timeout for the heartbeat in milliseconds
    /// @note This function sends a heartbeat message setting up the slave to hearing the master
    /// heartbeat. When the slave does not receive the heartbeat message within the timeout period,
    /// it will stop sending heartbeats and the master will not receive any more heartbeats from the
    /// slave. If the slave is not configured to send heartbeats, this function will not work and
    /// will return an error.
    void heartbeatTest(u32 MasterId, u32 SlaveId, u32 HeartbeatTimeout);

    /// @brief Send a sync message
    /// @param id The CAN ID of the device to send the sync message
    void SendSync(u16 id);

    /// @brief Load an EDS file
    /// @param edsFilePath The path to the EDS file to load
    /// @note This function will parse the EDS file and store the objects in memory for further
    /// operations.
    void edsLoad(const std::string& edsFilePath);

    /// @brief Unload the currently loaded EDS file
    void edsUnload();

    /// @brief Display the loaded EDS file
    /// @note This function will print the objects in the EDS file to the console.
    void edsDisplay();

    /// @brief Generate a markdown file from the loaded EDS file
    void edsGenerateMarkdown();

    /// @brief Generate an HTML file from the loaded EDS file
    void edsGenerateHtml();

    /// @brief Generate a C++ header and source file from the loaded EDS file
    /// @note The generated files can be included in the candletool project by switching the file
    /// OD.cpp & OD.hpp with generates ones.
    void edsGenerateCpp();

    /// @brief search for an object in the EDS file
    /// @param index The index of the object to search for
    /// @param subindex The subindex of the object to search for
    void edsGet(u32 index, u8 subindex);

    /// @brief search for an object in the EDS file
    /// @param research The string to search for in the EDS file
    void edsFind(const std::string& research);

    /// @brief Add an object to the EDS file
    /// @param obj The object to add to the EDS file
    void edsAddObject(const edsObject& obj);

    /// @brief Delete an object from the EDS file
    /// @param index The index of the object to delete
    /// @param subindex The subindex of the object to delete
    void edsDeleteObject(u32 index, u8 subindex);

    /// @brief Modify an object in the EDS file
    /// @param obj The new object to write in the EDS file
    /// @param id The CAN ID of the object to modify
    /// @param subIndex The sub-index of the object to modify
    void edsModifyCorrection(const edsObject& obj, u16 id, u8 subIndex);

    /// @brief Send a NMT command to the device
    /// @param id The CAN ID of the device to send the command to
    /// @param command The NMT command to send
    /// @note This function sends a NMT command to the device to control its state.
    void SendNMT(u8 id, u8 command);

    /// @brief Read the NMT state of the device
    /// @param id The CAN ID of the device to read the NMT state from
    /// @return The NMT state of the device
    void ReadHeartbeat(u16 id);

    // Retrieve the Candle object
    mab::Candle* getCandle()
    {
        return m_candle;
    }

    /// @brief Send a time samp message
    /// @param id
    void SendTime(uint16_t id);

  private:
    Logger       log;
    mab::Candle* m_candle;

    std::string busString;

    std::string validateAndGetFinalConfigPath(const std::string& cfgPath);

    u8 getNumericParamFromList(std::string& param, const std::vector<std::string>& list);

    template <class T>
    bool getField(mINI::INIStructure& cfg,
                  mINI::INIStructure& ini,
                  std::string         category,
                  std::string         field,
                  T&                  value);
    
    template <class T>
    bool checkParamLimit(T value, T min, T max)
    {
        if (value > max)
            return false;
        if (value < min)
            return false;
        return true;
    }

    bool checkSetupError(u16 id);
};
