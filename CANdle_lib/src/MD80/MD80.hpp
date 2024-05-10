#ifndef MD80_HPP
#define MD80_HPP

#include <cstdint>
#include <map>
#include <memory>
/* TODO replace with logger */
#include <iostream>

#include "CanopenStack/CANopen/CanopenStack.hpp"
#include "IObjectDictionaryParser.hpp"
#include "ObjectDictionaryParserEDS.hpp"

/**
 * @brief implements general methods for reading and writing data to MD80 drivers
 *
 */
class MD80
{
   public:
	/**
	 * @brief An enum holding possible modes of operations of the MD80 controller. Some fields are CiA402 - compliant.
	 *
	 */
	enum class ModesOfOperation : int8_t
	{
		IMPEDANCE = -3,			  /**< Impedance PD mode */
		SERVICE = -2,			  /**< Service is used for calibration routines and homing */
		IDLE = 0,				  /**< Idle is default motion after power up */
		PROFILE_POSITION = 1,	  /**< Profiled position mode */
		PROFILE_VELOCITY = 2,	  /**< Profiled veloctiy mode */
		CYCLIC_SYNC_POSITION = 8, /**< Simple position PID mode */
		CYCLIC_SYNC_VELOCTIY = 9, /**< Simple velocity PID mode */
	};

	/**
	 * @brief Routine ID - use to run a selected routine on the MD80 controller
	 *
	 */
	enum RoutineID
	{
		BLINK = 1,
		RESET,
		CALIBRATION,
		CALIBRATION_AUX,
		SET_ZERO,
		CALIBRATE_PI_GAINS,
		TEST_OUTPUT_ENCODER,
		TEST_MAIN_ENCODER,
		SAVE,
		REVERT_FACTORY_SETTINGS,
		CAN_REINIT,
		RUN_HOMING,
	};

	/**
	 * @brief Construct a new MD80 object.
	 *
	 * @param id ID of the drive.
	 * @param canopenStack std::shared_ptr to CanopenStack.
	 */
	MD80(uint32_t id, std::shared_ptr<CanopenStack> canopenStack);

	/**
	 * @brief Get output position.
	 *
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return float
	 */
	float getOutputPosition(bool useSDO = true);
	/**
	 * @brief Get the output velocity.
	 *
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return float
	 */
	float getOutputVelocity(bool useSDO = true);
	/**
	 * @brief Get the output torque.
	 *
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return float
	 */
	float getOutputTorque(bool useSDO = true);
	/**
	 * @brief Set target position.
	 *
	 * @param position target position to be set.
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return true
	 * @return false
	 */
	bool setPositionTarget(float position, bool useSDO = true);
	/**
	 * @brief Set target velocity.
	 *
	 * @param velocity target velocity to be set.
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return true
	 * @return false
	 */
	bool setVelocityTarget(float velocity, bool useSDO = true);
	/**
	 * @brief Set target torque.
	 *
	 * @param torque target torque to be set.
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return true
	 * @return false
	 */
	bool setTorqueTarget(float torque, bool useSDO = true);
	/**
	 * @brief Get quick status.
	 *
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return uint32_t quick status.
	 */
	uint32_t getQuickStatus(bool useSDO = true);
	/**
	 * @brief Check if a target value, depending on the currently active motion mode, is reached.
	 *
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return true
	 * @return false
	 */
	bool isTargetReached(bool useSDO = true);
	/**
	 * @brief Set velocity PID gains.
	 *
	 * @param kp proportional gain.
	 * @param ki integral gain.
	 * @param kd derivative gain.
	 * @param intLimit integral limit (anti-windup).
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return true
	 * @return false
	 */
	bool setVelocityPidGains(float kp, float ki, float kd, float intLimit, bool useSDO = true);
	/**
	 * @brief Set position PID gains.
	 *
	 * @param kp proportional gain.
	 * @param ki integral gain.
	 * @param kd derivative gain.
	 * @param intLimit integral limit (anti-windup).
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return true
	 * @return false
	 */
	bool setPositionPidGains(float kp, float ki, float kd, float intLimit, bool useSDO = true);
	/**
	 * @brief Set impedance PD gains.
	 *
	 * @param kp integral gain.
	 * @param kd derivative gain.
	 * @param useSDO set to false to use PDO access (faster but needs to be configured first).
	 * @return true
	 * @return false
	 */
	bool setImpedancePdGains(float kp, float kd, bool useSDO = true);
	/**
	 * @brief Run a routine. Select from RoutineID.
	 *
	 * @param routineId RoutineID id
	 * @param shouldWaitForCompletion set to true if the function must wait for routine completion.
	 * @return true
	 * @return false
	 */
	bool runRoutine(RoutineID routineId, bool shouldWaitForCompletion);
	/**
	 * @brief Enter operational mode. Only in operational mode non-service movement is allowed.
	 *
	 * @return true
	 * @return false
	 */
	bool enterOperational();
	/**
	 * @brief Enter switch on disabled mode. This is equivalent to exiting the operational mode.
	 *
	 * @return true
	 * @return false
	 */
	bool enterSwitchOnDisabled();
	/**
	 * @brief Set current mode of operation
	 *
	 * @param mode mode from ModesOfOperation enum
	 * @return true
	 * @return false
	 */
	bool setModeOfOperation(ModesOfOperation mode);
	/**
	 * @brief Setup fast PDO communication.
	 *
	 * @param pdoID ID of the TPDO or RPDO
	 * @param fields std::vector of index-subindex pairs that should be used in the selected PDO frame
	 * @return true
	 * @return false
	 */
	bool setupPDO(CanopenStack::PDO pdoID, const std::vector<std::pair<uint16_t, uint8_t>>& fields);

   public:
	IODParser::ODType OD;

   private:
	template <typename T>
	T readAccessHelper(uint16_t index, uint8_t subindex, bool useSDO)
	{
		if (!useSDO)
			return std::get<T>(OD.at(index)->subEntries.at(subindex)->value);

		T value = 0;
		uint32_t errorCode = 0;
		if (!canopenStack->readSDO(id, index, subindex, value, errorCode))
			std::cout << "Error reading SDO!" << std::endl;
		return value;
	}

	template <typename T>
	bool writeAccessHelper(uint16_t index, uint8_t subindex, T value, bool useSDO)
	{
		if (!useSDO)
		{
			OD.at(index)->subEntries.at(subindex)->value = value;
			return true;
		}

		uint32_t errorCode = 0;
		if (!canopenStack->writeSDO(id, index, subindex, value, errorCode))
			return false;
		return true;
	}

   private:
	uint32_t id;
	std::shared_ptr<CanopenStack> canopenStack;
};

#endif