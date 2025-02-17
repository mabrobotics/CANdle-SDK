
#include "register.hpp"

#include "candle.hpp"

namespace mab
{

    bool Register::prepare(uint16_t canId, mab::Md80FrameId_E frameType)
    {
        (void)frameType;
        /* clear the RX buffer and send register request */
        memset(regRxBuffer, 0, sizeof(regRxBuffer));
        bool status = candle->sendGenericFDCanFrame(
            canId, regTxPtr - regTxBuffer, regTxBuffer, regRxBuffer, nullptr, 10);
        regTxPtr = nullptr;
        regRxPtr = nullptr;
        return status;
    }

    bool Register::interpret(uint16_t canId)
    {
        (void)canId;
        return true;
    }

    uint32_t Register::pack(uint16_t regId, char* value, char* buffer)
    {
        uint32_t len = getSize(regId);
        /* in case no place is left in the buffer */
        if ((len + 2) > (sizeof(regTxBuffer) - (buffer - regTxBuffer)))
        {
            throw "Error while packaging data. Make sure its size is not above 62 bytes. Remember to add 2 bytes per field (field ID).";
            return 0;
        }
        /* place register ID at the beginning */
        *(uint16_t*)buffer = regId;
        /* move the pointer forward by 2 bytes */
        buffer += sizeof(regId);
        /* in case we're just preparing a read frame */
        if (value == nullptr)
            memset(buffer, 0, len);
        else
            memcpy(buffer, value, len);

        return (len + sizeof(regId));
    }

    uint32_t Register::unPack(uint16_t regId, char* value, char* buffer)
    {
        /* place register ID at the beginning */
        *(uint16_t*)buffer = regId;
        /* move the pointer forward by 2 bytes */
        buffer += sizeof(regId);

        uint32_t len = getSize(regId);

        return copy(value, buffer, len, sizeof(regTxBuffer) - (buffer - &regTxBuffer[2]));
    }

    uint32_t Register::copy(char* dest, char* source, uint32_t size, uint32_t freeSpace)
    {
        /* return two so that we move by the reg ID and find a zero reg ID which terminates
         * reception/transmission */
        if (freeSpace < size)
            return 0;

        memcpy(dest, source, size);
        return size + 2;
    }

    bool Register::prepareFrame(mab::Md80FrameId_E frameId, mdRegister_E regId, char* value)
    {
        /* if new frame */
        if (regTxPtr == nullptr)
        { /* clear the buffer */
            memset(regTxBuffer, 0, sizeof(regTxBuffer));
            regTxBuffer[0] = frameId;
            regTxBuffer[1] = 0;
            regTxPtr       = &regTxBuffer[2];
        }
        /* let pack know to fill data space with zeros */
        if (frameId == mab::Md80FrameId_E::FRAME_READ_REGISTER)
            value = nullptr;
        /* add value's data to tx buffer */
        uint32_t offset = pack(regId, value, regTxPtr);

        if (offset == 0)
        {
            throw "Error while packaging data. Make sure its size is not above 62 bytes. Remember to add 2 bytes per field (field ID).";
            return false;
        }

        regTxPtr += offset;
        return true;
    }

    uint16_t Register::getSize(uint16_t regId)
    {
        if (regId == mdRegister_E::commitHash)
            return 8;
        if (regId == mdRegister_E::motorName)
            return 24;

        switch (getType(regId))
        {
            case type::I8:
            case type::U8:
                return 1;
            case type::I16:
            case type::U16:
                return 2;
            case type::I32:
            case type::U32:
            case type::F32:
                return 4;
            case type::REGARR:
            case type::STR:
            case type::UNKNOWN:
                return 0;
        }
        return 0;
    }
    // TODO: THIS SECTION NEEDS REWORK IN THE FUTURE
    Register::type Register::getType(uint16_t regId)
    {
        switch (regId)
        {
            case mdRegister_E::reverseDirection:
            case mdRegister_E::motionModeStatus:
            case mdRegister_E::motionModeCommand:
            case mdRegister_E::homingMode:
            case mdRegister_E::motorThermistorType:
            case mdRegister_E::motorCalibrationMode:
            case mdRegister_E::outputEncoderCalibrationMode:
            case mdRegister_E::outputEncoderMode:
            case mdRegister_E::bridgeType:
            case mdRegister_E::outputEncoder:
            case mdRegister_E::legacyHardwareVersion:
            case mdRegister_E::canTermination:
            case mdRegister_E::motorShutdownTemp:
            case mdRegister_E::runCalibrateCmd:
            case mdRegister_E::runCalibrateAuxEncoderCmd:
            case mdRegister_E::runCalibratePiGains:
            case mdRegister_E::runtestAuxEncoderCmd:
            case mdRegister_E::runTestMainEncoderCmd:
            case mdRegister_E::runSaveCmd:
            case mdRegister_E::runHoming:
            case mdRegister_E::runRestoreFactoryConfig:
            case mdRegister_E::runReset:
            case mdRegister_E::runClearWarnings:
            case mdRegister_E::runClearErrors:
            case mdRegister_E::runBlink:
            case mdRegister_E::runZero:
            case mdRegister_E::runCanReinit:
            case mdRegister_E::userGpioConfiguration:
                return type::U8;
            case mdRegister_E::motorTorqueBandwidth:
            case mdRegister_E::canWatchdog:
            case mdRegister_E::quickStatus:
            case mdRegister_E::motorKV:
            case mdRegister_E::state:
            case mdRegister_E::hardwareType:
                return type::U16;
            case mdRegister_E::outputEncoderDefaultBaud:
            case mdRegister_E::canBaudrate:
            case mdRegister_E::canId:
            case mdRegister_E::motorPolePairs:
            case mdRegister_E::mainEncoderStatus:
            case mdRegister_E::auxEncoderStatus:
            case mdRegister_E::calibrationStatus:
            case mdRegister_E::bridgeStatus:
            case mdRegister_E::hardwareStatus:
            case mdRegister_E::miscStatus:
            case mdRegister_E::communicationStatus:
            case mdRegister_E::homingStatus:
            case mdRegister_E::motionStatus:
            case mdRegister_E::firmwareVersion:
            case mdRegister_E::buildDate:
                return type::U32;
            case mdRegister_E::targetPosition:
            case mdRegister_E::targetVelocity:
            case mdRegister_E::targetTorque:
            case mdRegister_E::motorTorque:
            case mdRegister_E::positionWindow:
            case mdRegister_E::velocityWindow:
            case mdRegister_E::maxTorque:
            case mdRegister_E::maxVelocity:
            case mdRegister_E::quickStopDeceleration:
            case mdRegister_E::profileAcceleration:
            case mdRegister_E::profileDeceleration:
            case mdRegister_E::profileVelocity:
            case mdRegister_E::homingMaxTravel:
            case mdRegister_E::homingVelocity:
            case mdRegister_E::homingTorque:
            case mdRegister_E::positionLimitMax:
            case mdRegister_E::positionLimitMin:
            case mdRegister_E::shuntResistance:
            case mdRegister_E::maxAcceleration:
            case mdRegister_E::maxDeceleration:
            case mdRegister_E::mainEncoderVelocity:
            case mdRegister_E::mainEncoderPosition:
            case mdRegister_E::mosfetTemperature:
            case mdRegister_E::motorTemperature:
            case mdRegister_E::motorInductance:
            case mdRegister_E::motorResistance:
            case mdRegister_E::motorImpPidKp:
            case mdRegister_E::motorImpPidKd:
            case mdRegister_E::motorPosPidKp:
            case mdRegister_E::motorPosPidKi:
            case mdRegister_E::motorPosPidKd:
            case mdRegister_E::motorPosPidWindup:
            case mdRegister_E::motorVelPidKp:
            case mdRegister_E::motorVelPidKi:
            case mdRegister_E::motorVelPidKd:
            case mdRegister_E::motorVelPidWindup:
            case mdRegister_E::motorFriction:
            case mdRegister_E::motorStriction:
            case mdRegister_E::outputEncoderDir:
            case mdRegister_E::outputEncoderVelocity:
            case mdRegister_E::outputEncoderPosition:
            case mdRegister_E::motorGearRatio:
            case mdRegister_E::calOutputEncoderStdDev:
            case mdRegister_E::calOutputEncoderMinE:
            case mdRegister_E::calOutputEncoderMaxE:
            case mdRegister_E::calMainEncoderStdDev:
            case mdRegister_E::calMainEncoderMinE:
            case mdRegister_E::calMainEncoderMaxE:
            case mdRegister_E::motorKt:
            case mdRegister_E::motorKt_a:
            case mdRegister_E::motorKt_b:
            case mdRegister_E::motorKt_c:
            case mdRegister_E::motorMaxCurrent:
                return type::F32;
            case mdRegister_E::commitHash:
            case mdRegister_E::motorName:
                return type::STR;
            case mdRegister_E::defaultResponse:
            case mdRegister_E::defaultCommand:
                return type::REGARR;
            default:
                return type::UNKNOWN;
        }
    }

}  // namespace mab
