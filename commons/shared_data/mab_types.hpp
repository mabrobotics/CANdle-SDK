#pragma once

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

typedef uint64_t u64;
typedef int64_t  s64;
typedef int64_t  i64;

typedef uint32_t u32;
typedef int32_t  s32;
typedef int32_t  i32;

typedef uint16_t u16;
typedef int16_t  s16;
typedef int16_t  i16;

typedef uint8_t u8;
typedef int8_t  s8;
typedef int8_t  i8;

typedef float f32;

typedef void function(void*);
typedef u32  flags_t;

#ifdef __cplusplus
namespace mab
{
    using canId_t = u16;

    /// @brief CAN bus baudrates
    enum CANdleBaudrate_E : uint8_t
    {
        CAN_BAUD_1M = 1, /*!< FDCAN Baudrate of 1Mbps (1 000 000 bits per second) */
        CAN_BAUD_2M = 2, /*!< FDCAN Baudrate of 2Mbps (2 000 000 bits per second) */
        CAN_BAUD_5M = 5, /*!< FDCAN Baudrate of 5Mbps (5 000 000 bits per second) */
        CAN_BAUD_8M = 8, /*!< FDCAN Baudrate of 8Mbps (8 000 000 bits per second) */
    };
    /**
     * @brief Impedance controller parameters
     *
     * Impedance controller output is computed as: torque = kp * position_error + kd *
     * velocity_error + torque_ff;
     */
    struct RegImpedance_t
    {
        float kp;
        float kd;
        float torque_ff;
    };
    /**
     * @brief PID controller parameters. This is used to setup either Position PID controller or
     * Velocity PID controller.
     * @note i_windup is an anti-windup parameter. This limits the maximum output of the integral
     * (i) part of the controller.
     */
    struct RegPid_t
    {
        float kp, ki, kd, i_windup;
    };

    /**
     * @brief MD Control Mode
     * @note Position PID is a cascade controller, output of the Position PID (target velocity) is
     * passed as an input of Velocity PID. Velocity PID output (torque) is then passed directly to
     * internal current/torque controller.
     */
    enum MdMode_E : uint8_t
    {
        IDLE         = 0, /*!< Idle mode, no control output */
        POSITION_PID = 1, /*!< Position PID mode (cascade controllers) */
        VELOCITY_PID = 2, /*!< Velocity PID mode */
        RAW_TORQUE   = 3, /*!< Raw torque mode */
        IMPEDANCE =
            4, /*!< Impedance mode, uses Impedance controller similar to spring-damper system */
        POSITION_PROFILE = 7, /*!< Position PID with trapezoidal profile (constant acceleration) */
        VELOCITY_PROFILE = 8, /*!< Velocity PID with trapezoidal profile (constant acceleration) */
    };

    enum BusDeviceType_E : uint8_t
    {
        DEVICE_TYPE_UNKNOWN = 0x00,
        DEVICE_TYPE_MD      = 0x01,
        DEVICE_TYPE_PDS     = 0x02
    };

    struct BusDevice_S
    {
        uint16_t        canId;
        BusDeviceType_E type;
    };

    struct CanFrame_t
    {
        uint8_t length;
        uint8_t data[32];
    };

    // ########################

    typedef struct
    {
        float kp;
        float kd;
        float outMax;
    } ImpedanceControllerGains_t;

    typedef struct
    {
        float kp;
        float ki;
        float kd;
        float intWindup;
        float outMax;
    } PidControllerGains_t;

    typedef union version_ut
    {
        struct
        {
            char    tag;
            uint8_t revision;
            uint8_t minor;
            uint8_t major;
        } s;
        u32 i;
    } version_ut;

}  // namespace mab

#endif
