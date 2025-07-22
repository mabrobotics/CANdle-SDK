/*******************************************************************************
    CANopen Object Dictionary definition for CANopenNode V4

    This file was automatically generated with
    libedssharp Object Dictionary Editor v0.8-125-g85dfd8e

    https://github.com/CANopenNode/CANopenNode
    https://github.com/robincornelius/libedssharp

    DON'T EDIT THIS FILE MANUALLY !!!!
********************************************************************************

    File info:
        File Names:   OD.h; OD.c
        Project File: MDv1.0.0.eds
        File Version: 1.0

        Created:      11/23/2020 1:00:00 PM
        Created By:   Piotr Wasilewski
        Modified:     6/11/2024 11:08:00 AM
        Modified By:  Pawel Korsak

    Device Info:
        Vendor Name:  MAB ROBOTICS
        Vendor ID:    1
        Product Name: MD
        Product ID:   1

        Description:  MD motor controller
*******************************************************************************/

#include <stdbool.h>

#ifndef OD_H
#define OD_H
/*******************************************************************************
    Counters of OD objects
*******************************************************************************/
#define OD_CNT_NMT       1
#define OD_CNT_EM        1
#define OD_CNT_SYNC      1
#define OD_CNT_SYNC_PROD 1
#define OD_CNT_STORAGE   1
#define OD_CNT_TIME      1
#define OD_CNT_EM_PROD   1
#define OD_CNT_HB_CONS   1
#define OD_CNT_HB_PROD   1
#define OD_CNT_SDO_SRV   1
#define OD_CNT_SDO_CLI   1
#define OD_CNT_RPDO      4
#define OD_CNT_TPDO      4

/*******************************************************************************
    Sizes of OD arrays
*******************************************************************************/
#define OD_CNT_ARR_1003 16
#define OD_CNT_ARR_1010 4
#define OD_CNT_ARR_1011 4
#define OD_CNT_ARR_1016 8

/*******************************************************************************
    OD data declaration of all groups
*******************************************************************************/

// ----------------------------------------------------------------------------
// Déclaration minimale de OD_entry_t et des types d’objet CANopenNode
// ----------------------------------------------------------------------------
typedef struct
{
    unsigned int   index;       // index CANopen (0x1000, 0x1001, etc.)
    unsigned short subIndex;    // sous-index (0x00 … 0xFF)
    unsigned short objectType;  // ODT_VAR, ODT_ARR ou ODT_REC
    void*          dataPtr;     // pointeur vers la donnée
    void*          pFunct;      // pointeur vers une fonction (ou NULL)
} OD_entry_t;

// Types d’objet pour OD_entry_t.objectType
#define ODT_VAR 0x01  // variable simple
#define ODT_ARR 0x02  // tableau
#define ODT_REC 0x03  // enregistrement (record)

// -----------------------------
// Types de base OD (si non déjà définis)
// -----------------------------
#ifndef CO_OD_OWN_TYPES
typedef unsigned long  OD_size_t;  // taille de données
typedef unsigned short OD_attr_t;  // bitfield d'attributs
#endif

typedef struct
{
    void*     dataOrig;   /**< Pointer to data */
    OD_attr_t attribute;  /**< Attribute bitfield, see @ref OD_attributes_t */
    OD_size_t dataLength; /**< Data length in bytes */
} OD_obj_var_t;

/**
 * Object for OD array of variables, used for "ARRAY" type OD objects
 */
typedef struct
{
    unsigned short* dataOrig0;  /**< Pointer to data for sub-index 0 */
    void*           dataOrig;   /**< Pointer to array of data */
    OD_attr_t       attribute0; /**< Attribute bitfield for sub-index 0, see @ref OD_attributes_t */
    OD_attr_t       attribute;  /**< Attribute bitfield for array elements */
    OD_size_t       dataElementLength; /**< Data length of array elements in bytes */
    OD_size_t       dataElementSizeof; /**< Sizeof one array element in bytes */
} OD_obj_array_t;

/**
 * Object for OD sub-elements, used in "RECORD" type OD objects
 */
typedef struct
{
    void*          dataOrig;   /**< Pointer to data */
    unsigned short subIndex;   /**< Sub index of element. */
    OD_attr_t      attribute;  /**< Attribute bitfield, see @ref OD_attributes_t */
    OD_size_t      dataLength; /**< Data length in bytes */
} OD_obj_record_t;

typedef struct
{
    unsigned long  x1000_deviceType;
    unsigned long  x1005_COB_ID_SYNCMessage;
    unsigned long  x1006_communicationCyclePeriod;
    unsigned long  x1007_synchronousWindowLength;
    char           x1008_manufacturerDeviceName[5];
    char           x1009_manufacturerHardwareVersion[2];
    unsigned long  x1012_COB_IDTimeStampObject;
    unsigned long  x1014_COB_ID_EMCY;
    unsigned int   x1015_inhibitTimeEMCY;
    unsigned short x1016_consumerHeartbeatTime_sub0;
    unsigned long  x1016_consumerHeartbeatTime[8];
    unsigned int   x1017_producerHeartbeatTime;
    struct
    {
        unsigned short highestSub_indexSupported;
        unsigned long  vendor_ID;
        unsigned long  productCode;
        unsigned long  revisionNumber;
        unsigned long  serialNumber;
    } x1018_identity;
    unsigned short x1019_synchronousCounterOverflowValue;
    struct
    {
        unsigned short highestSub_indexSupported;
        unsigned long  COB_IDClientToServerTx;
        unsigned long  COB_IDServerToClientRx;
        unsigned short node_IDOfTheSDOServer;
    } x1280_SDOClientParameter;
    struct
    {
        unsigned short maxSub_index;
        unsigned long  COB_IDUsedByRPDO;
        unsigned short transmissionType;
    } x1400_RPDOCommunicationParameter;
    struct
    {
        unsigned short maxSub_index;
        unsigned long  COB_IDUsedByRPDO;
        unsigned short transmissionType;
    } x1401_RPDOCommunicationParameter;
    struct
    {
        unsigned short maxSub_index;
        unsigned long  COB_IDUsedByRPDO;
        unsigned short transmissionType;
    } x1402_RPDOCommunicationParameter;
    struct
    {
        unsigned short maxSub_index;
        unsigned long  COB_IDUsedByRPDO;
        unsigned short transmissionType;
    } x1403_RPDOCommunicationParameter;
    struct
    {
        unsigned short numberOfMappedObjects;
        unsigned long  mappedObject_1;
        unsigned long  mappedObject_2;
        unsigned long  mappedObject_3;
        unsigned long  mappedObject_4;
        unsigned long  mappedObject_5;
        unsigned long  mappedObject_6;
        unsigned long  mappedObject_7;
        unsigned long  mappedObject_8;
    } x1600_RPDOMappingParameter;
    struct
    {
        unsigned short numberOfMappedObjects;
        unsigned long  mappedObject_1;
        unsigned long  mappedObject_2;
        unsigned long  mappedObject_3;
        unsigned long  mappedObject_4;
        unsigned long  mappedObject_5;
        unsigned long  mappedObject_6;
        unsigned long  mappedObject_7;
        unsigned long  mappedObject_8;
    } x1601_RPDOMappingParameter;
    struct
    {
        unsigned short numberOfMappedObjects;
        unsigned long  mappedObject_1;
        unsigned long  mappedObject_2;
        unsigned long  mappedObject_3;
        unsigned long  mappedObject_4;
        unsigned long  mappedObject_5;
        unsigned long  mappedObject_6;
        unsigned long  mappedObject_7;
        unsigned long  mappedObject_8;
    } x1602_RPDOMappingParameter;
    struct
    {
        unsigned short numberOfMappedObjects;
        unsigned long  mappedObject_1;
        unsigned long  mappedObject_2;
        unsigned long  mappedObject_3;
        unsigned long  mappedObject_4;
        unsigned long  mappedObject_5;
        unsigned long  mappedObject_6;
        unsigned long  mappedObject_7;
        unsigned long  mappedObject_8;
    } x1603_RPDOMappingParameter;
    struct
    {
        unsigned short maxSub_index;
        unsigned long  COB_IDUsedByTPDO;
        unsigned short transmissionType;
        unsigned int   inhibitTime;
        unsigned short compatibilityEntry;
        unsigned int   eventTimer;
        unsigned short SYNCStartValue;
    } x1800_TPDOCommunicationParameter;
    struct
    {
        unsigned short maxSub_index;
        unsigned long  COB_IDUsedByTPDO;
        unsigned short transmissionType;
        unsigned int   inhibitTime;
        unsigned short compatibilityEntry;
        unsigned int   eventTimer;
        unsigned short SYNCStartValue;
    } x1801_TPDOCommunicationParameter;
    struct
    {
        unsigned short maxSub_index;
        unsigned long  COB_IDUsedByTPDO;
        unsigned short transmissionType;
        unsigned int   inhibitTime;
        unsigned short compatibilityEntry;
        unsigned int   eventTimer;
        unsigned short SYNCStartValue;
    } x1802_TPDOCommunicationParameter;
    struct
    {
        unsigned short maxSub_index;
        unsigned long  COB_IDUsedByTPDO;
        unsigned short transmissionType;
        unsigned int   inhibitTime;
        unsigned short compatibilityEntry;
        unsigned int   eventTimer;
        unsigned short SYNCStartValue;
    } x1803_TPDOCommunicationParameter;
    struct
    {
        unsigned short numberOfMappedObjects;
        unsigned long  mappedObject_1;
        unsigned long  mappedObject_2;
    } x1A00_TPDOMappingParameter;
    struct
    {
        unsigned short numberOfMappedObjects;
        unsigned long  mappedObject_1;
        unsigned long  mappedObject_2;
    } x1A01_TPDOMappingParameter;
    struct
    {
        unsigned short numberOfMappedObjects;
        unsigned long  mappedObject_1;
        unsigned long  mappedObject_2;
    } x1A02_TPDOMappingParameter;
    struct
    {
        unsigned short numberOfMappedObjects;
        unsigned long  mappedObject_1;
        unsigned long  mappedObject_2;
    } x1A03_TPDOMappingParameter;
} OD_PERSIST_COMM_t;

typedef struct
{
    unsigned short x1001_errorRegister;
    unsigned short x1010_storeParameters_sub0;
    unsigned long  x1010_storeParameters[4];
    unsigned short x1011_restoreDefaultParameters_sub0;
    unsigned long  x1011_restoreDefaultParameters[4];
    struct
    {
        unsigned short highestSub_indexSupported;
        unsigned long  COB_IDClientToServerRx;
        unsigned long  COB_IDServerToClientTx;
    } x1200_SDOServerParameter;
    struct
    {
        unsigned short highestSub_indexSupported;
        unsigned long  COB_IDClientToServerRx;
        unsigned long  COB_IDServerToClientTx;
    } x1201_SDOServerParameter;
    struct
    {
        unsigned short highestSub_indexSupported;
        unsigned long  polePairs;
        double         torqueConstant;
        double         phaseInductance;
        double         phaseResistance;
        unsigned int   torqueBandwidth;
        char           motorName[5];
        unsigned short motorShutdownTemperature;
        double         gearRatio;
        unsigned short calibrationMode;
        unsigned long  canID;
        unsigned long  canBaudrate;
        unsigned int   canWatchdog;
        bool           reverseDirection;
    } x2000_motorSettings;
    struct
    {
        unsigned short highestSub_indexSupported;
        double         kp;
        double         ki;
        double         kd;
        double         integralLimit;
    } x2001_velocityPID_Controller;
    struct
    {
        unsigned short highestSub_indexSupported;
        double         kp;
        double         ki;
        double         kd;
        double         integralLimit;
    } x2002_positionPID_Controller;
    struct
    {
        unsigned short highestSub_indexSupported;
        bool           blinkLEDs;
        bool           resetController;
        bool           runCalibration;
        bool           runOutputEncoderCalibration;
        bool           setZero;
        bool           calibrateCurrentPI_Gains;
        bool           testOutputEncoder;
        bool           testMainEncoder;
        bool           saveMemory;
        bool           revertFactorySettings;
        bool           clearErrors;
        bool           clearWarnings;
        bool           runCanReinit;
    } x2003_systemCommand;
    struct
    {
        unsigned short highestSub_indexSupported;
        unsigned long  mainEncoderStatus;
        unsigned long  outputEncoderStatus;
        unsigned long  calibrationStatus;
        unsigned long  bridgeStatus;
        unsigned long  hardwareStatus;
        unsigned long  homingStatus;
        unsigned long  motionStatus;
        unsigned long  communicationStatus;
    } x2004_systemStatus;
    struct
    {
        unsigned short highestSub_indexSupported;
        unsigned short type;
        unsigned short calibrationMode;
        unsigned short mode;
        double         position;
        double         velocity;
    } x2005_outputEncoder;
    struct
    {
        unsigned short highestSub_indexSupported;
        double         motorTemperature;
        double         mosfetTemperature;
    } x2006_temperature;
    struct
    {
        unsigned short highestSub_indexSupported;
        double         maxTorque;
        double         maxAcceleration;
        double         maxDeceleration;
        double         maxVelocity;
        double         maxPosition;
        double         minPosition;
    } x2007_limits;
    struct
    {
        unsigned short highestSub_indexSupported;
        unsigned short modeCommand;
        unsigned short modeStatus;
        double         profileVelocity;
        double         profileAcceleration;
        double         profileDeceleration;
        double         quickStopDeceleration;
        double         positionWindow;
        double         velocityWindow;
        double         targetPosition;
        double         targetVelocity;
        double         targetTorque;
    } x2008_motion;
    struct
    {
        unsigned short highestSub_indexSupported;
        double         outputPosition;
        double         outputVelocity;
        double         outputTorque;
    } x2009_motorMesurements;
    struct
    {
        unsigned short highestSub_indexSupported;
        char           commitHash[2];
        unsigned long  buildDate;
        unsigned long  version;
    } x200A_firmwareInfo;
    struct
    {
        unsigned short highestSub_indexSupported;
        bool           bootloaderFixed;
        char           commitHash[2];
        unsigned long  buildDate;
        unsigned long  version;
    } x200B_bootloderInfo;
    struct
    {
        unsigned short highestSub_indexSupported;
        double         kp;
        double         kd;
    } x200C_impedancePD_Controller;
    struct
    {
        unsigned short highestSub_indexSupported;
        unsigned short userGPIO_Configuration;
        unsigned int   userGPIO_State;
    } x200D_userGPIO;
    double        x200E_DCBusVoltage;
    unsigned int  x6040_controlword;
    unsigned int  x6041_statusword;
    short         x6060_modesOfOperation;
    short         x6061_modesOfOperationDisplay;
    long          x6062_positionDemandValue;
    long          x6064_positionValue;
    unsigned long x6067_positionWindow;
    long          x606B_velocityDemandValue;
    long          x606C_velocityActualValue;
    unsigned long x606D_velocityWindow;
    unsigned int  x6072_maxTorque;
    unsigned int  x6073_maxCurrent;
    int           x6074_torqueDemandValue;
    unsigned long x6075_motorRatedCurrent;
    unsigned long x6076_motorRatedTorque;
    int           x6077_torqueActualValue;
    unsigned long x6079_DClinkCircuitVoltage;
    long          x607A_targetPosition;
    struct
    {
        unsigned short highestSub_indexSupported;
        long           minPositionLimit;
        long           maxPositionLimit;
    } x607D_softwarePositionLimit;
    unsigned long x6080_maxMotorSpeed;
    unsigned long x6081_profileVelocity;
    unsigned long x6083_profileAcceleration;
    unsigned long x6084_profileDeceleration;
    unsigned long x6085_quickStopDeceleration;
    unsigned long x60C5_maxAcceleration;
    unsigned long x60C6_maxDeceleration;
    long          x60FF_targetVelocity;
    unsigned long x6502_supportedDriveModes;
} OD_RAM_t;

extern OD_PERSIST_COMM_t OD_PERSIST_COMM;
extern OD_RAM_t          OD_RAM;

#ifndef CO_PROGMEM
#define CO_PROGMEM
#endif
extern OD_entry_t CO_PROGMEM ODList[];

// extern OD_t*             OD;

/*******************************************************************************
    Object dictionary entries - shortcuts
*******************************************************************************/
#define OD_ENTRY_H1000 &OD->list[0]
#define OD_ENTRY_H1001 &OD->list[1]
#define OD_ENTRY_H1003 &OD->list[2]
#define OD_ENTRY_H1005 &OD->list[3]
#define OD_ENTRY_H1006 &OD->list[4]
#define OD_ENTRY_H1007 &OD->list[5]
#define OD_ENTRY_H1008 &OD->list[6]
#define OD_ENTRY_H1009 &OD->list[7]
#define OD_ENTRY_H1010 &OD->list[8]
#define OD_ENTRY_H1011 &OD->list[9]
#define OD_ENTRY_H1012 &OD->list[10]
#define OD_ENTRY_H1014 &OD->list[11]
#define OD_ENTRY_H1015 &OD->list[12]
#define OD_ENTRY_H1016 &OD->list[13]
#define OD_ENTRY_H1017 &OD->list[14]
#define OD_ENTRY_H1018 &OD->list[15]
#define OD_ENTRY_H1019 &OD->list[16]
#define OD_ENTRY_H1200 &OD->list[17]
#define OD_ENTRY_H1201 &OD->list[18]
#define OD_ENTRY_H1280 &OD->list[19]
#define OD_ENTRY_H1400 &OD->list[20]
#define OD_ENTRY_H1401 &OD->list[21]
#define OD_ENTRY_H1402 &OD->list[22]
#define OD_ENTRY_H1403 &OD->list[23]
#define OD_ENTRY_H1600 &OD->list[24]
#define OD_ENTRY_H1601 &OD->list[25]
#define OD_ENTRY_H1602 &OD->list[26]
#define OD_ENTRY_H1603 &OD->list[27]
#define OD_ENTRY_H1800 &OD->list[28]
#define OD_ENTRY_H1801 &OD->list[29]
#define OD_ENTRY_H1802 &OD->list[30]
#define OD_ENTRY_H1803 &OD->list[31]
#define OD_ENTRY_H1A00 &OD->list[32]
#define OD_ENTRY_H1A01 &OD->list[33]
#define OD_ENTRY_H1A02 &OD->list[34]
#define OD_ENTRY_H1A03 &OD->list[35]
#define OD_ENTRY_H2000 &OD->list[36]
#define OD_ENTRY_H2001 &OD->list[37]
#define OD_ENTRY_H2002 &OD->list[38]
#define OD_ENTRY_H2003 &OD->list[39]
#define OD_ENTRY_H2004 &OD->list[40]
#define OD_ENTRY_H2005 &OD->list[41]
#define OD_ENTRY_H2006 &OD->list[42]
#define OD_ENTRY_H2007 &OD->list[43]
#define OD_ENTRY_H2008 &OD->list[44]
#define OD_ENTRY_H2009 &OD->list[45]
#define OD_ENTRY_H200A &OD->list[46]
#define OD_ENTRY_H200B &OD->list[47]
#define OD_ENTRY_H200C &OD->list[48]
#define OD_ENTRY_H200D &OD->list[49]
#define OD_ENTRY_H200E &OD->list[50]
#define OD_ENTRY_H6040 &OD->list[51]
#define OD_ENTRY_H6041 &OD->list[52]
#define OD_ENTRY_H6060 &OD->list[53]
#define OD_ENTRY_H6061 &OD->list[54]
#define OD_ENTRY_H6062 &OD->list[55]
#define OD_ENTRY_H6064 &OD->list[56]
#define OD_ENTRY_H6067 &OD->list[57]
#define OD_ENTRY_H606B &OD->list[58]
#define OD_ENTRY_H606C &OD->list[59]
#define OD_ENTRY_H606D &OD->list[60]
#define OD_ENTRY_H6072 &OD->list[61]
#define OD_ENTRY_H6073 &OD->list[62]
#define OD_ENTRY_H6074 &OD->list[63]
#define OD_ENTRY_H6075 &OD->list[64]
#define OD_ENTRY_H6076 &OD->list[65]
#define OD_ENTRY_H6077 &OD->list[66]
#define OD_ENTRY_H6079 &OD->list[67]
#define OD_ENTRY_H607A &OD->list[68]
#define OD_ENTRY_H607D &OD->list[69]
#define OD_ENTRY_H6080 &OD->list[70]
#define OD_ENTRY_H6081 &OD->list[71]
#define OD_ENTRY_H6083 &OD->list[72]
#define OD_ENTRY_H6084 &OD->list[73]
#define OD_ENTRY_H6085 &OD->list[74]
#define OD_ENTRY_H60C5 &OD->list[75]
#define OD_ENTRY_H60C6 &OD->list[76]
#define OD_ENTRY_H60FF &OD->list[77]
#define OD_ENTRY_H6502 &OD->list[78]

/*******************************************************************************
    Object dictionary entries - shortcuts with names
*******************************************************************************/
#define OD_ENTRY_H1000_deviceType                      &OD->list[0]
#define OD_ENTRY_H1001_errorRegister                   &OD->list[1]
#define OD_ENTRY_H1003_pre_definedErrorField           &OD->list[2]
#define OD_ENTRY_H1005_COB_ID_SYNCMessage              &OD->list[3]
#define OD_ENTRY_H1006_communicationCyclePeriod        &OD->list[4]
#define OD_ENTRY_H1007_synchronousWindowLength         &OD->list[5]
#define OD_ENTRY_H1008_manufacturerDeviceName          &OD->list[6]
#define OD_ENTRY_H1009_manufacturerHardwareVersion     &OD->list[7]
#define OD_ENTRY_H1010_storeParameters                 &OD->list[8]
#define OD_ENTRY_H1011_restoreDefaultParameters        &OD->list[9]
#define OD_ENTRY_H1012_COB_IDTimeStampObject           &OD->list[10]
#define OD_ENTRY_H1014_COB_ID_EMCY                     &OD->list[11]
#define OD_ENTRY_H1015_inhibitTimeEMCY                 &OD->list[12]
#define OD_ENTRY_H1016_consumerHeartbeatTime           &OD->list[13]
#define OD_ENTRY_H1017_producerHeartbeatTime           &OD->list[14]
#define OD_ENTRY_H1018_identity                        &OD->list[15]
#define OD_ENTRY_H1019_synchronousCounterOverflowValue &OD->list[16]
#define OD_ENTRY_H1200_SDOServerParameter              &OD->list[17]
#define OD_ENTRY_H1201_SDOServerParameter              &OD->list[18]
#define OD_ENTRY_H1280_SDOClientParameter              &OD->list[19]
#define OD_ENTRY_H1400_RPDOCommunicationParameter      &OD->list[20]
#define OD_ENTRY_H1401_RPDOCommunicationParameter      &OD->list[21]
#define OD_ENTRY_H1402_RPDOCommunicationParameter      &OD->list[22]
#define OD_ENTRY_H1403_RPDOCommunicationParameter      &OD->list[23]
#define OD_ENTRY_H1600_RPDOMappingParameter            &OD->list[24]
#define OD_ENTRY_H1601_RPDOMappingParameter            &OD->list[25]
#define OD_ENTRY_H1602_RPDOMappingParameter            &OD->list[26]
#define OD_ENTRY_H1603_RPDOMappingParameter            &OD->list[27]
#define OD_ENTRY_H1800_TPDOCommunicationParameter      &OD->list[28]
#define OD_ENTRY_H1801_TPDOCommunicationParameter      &OD->list[29]
#define OD_ENTRY_H1802_TPDOCommunicationParameter      &OD->list[30]
#define OD_ENTRY_H1803_TPDOCommunicationParameter      &OD->list[31]
#define OD_ENTRY_H1A00_TPDOMappingParameter            &OD->list[32]
#define OD_ENTRY_H1A01_TPDOMappingParameter            &OD->list[33]
#define OD_ENTRY_H1A02_TPDOMappingParameter            &OD->list[34]
#define OD_ENTRY_H1A03_TPDOMappingParameter            &OD->list[35]
#define OD_ENTRY_H2000_motorSettings                   &OD->list[36]
#define OD_ENTRY_H2001_velocityPID_Controller          &OD->list[37]
#define OD_ENTRY_H2002_positionPID_Controller          &OD->list[38]
#define OD_ENTRY_H2003_systemCommand                   &OD->list[39]
#define OD_ENTRY_H2004_systemStatus                    &OD->list[40]
#define OD_ENTRY_H2005_outputEncoder                   &OD->list[41]
#define OD_ENTRY_H2006_temperature                     &OD->list[42]
#define OD_ENTRY_H2007_limits                          &OD->list[43]
#define OD_ENTRY_H2008_motion                          &OD->list[44]
#define OD_ENTRY_H2009_motorMesurements                &OD->list[45]
#define OD_ENTRY_H200A_firmwareInfo                    &OD->list[46]
#define OD_ENTRY_H200B_bootloderInfo                   &OD->list[47]
#define OD_ENTRY_H200C_impedancePD_Controller          &OD->list[48]
#define OD_ENTRY_H200D_userGPIO                        &OD->list[49]
#define OD_ENTRY_H200E_DCBusVoltage                    &OD->list[50]
#define OD_ENTRY_H6040_controlword                     &OD->list[51]
#define OD_ENTRY_H6041_statusword                      &OD->list[52]
#define OD_ENTRY_H6060_modesOfOperation                &OD->list[53]
#define OD_ENTRY_H6061_modesOfOperationDisplay         &OD->list[54]
#define OD_ENTRY_H6062_positionDemandValue             &OD->list[55]
#define OD_ENTRY_H6064_positionValue                   &OD->list[56]
#define OD_ENTRY_H6067_positionWindow                  &OD->list[57]
#define OD_ENTRY_H606B_velocityDemandValue             &OD->list[58]
#define OD_ENTRY_H606C_velocityActualValue             &OD->list[59]
#define OD_ENTRY_H606D_velocityWindow                  &OD->list[60]
#define OD_ENTRY_H6072_maxTorque                       &OD->list[61]
#define OD_ENTRY_H6073_maxCurrent                      &OD->list[62]
#define OD_ENTRY_H6074_torqueDemandValue               &OD->list[63]
#define OD_ENTRY_H6075_motorRatedCurrent               &OD->list[64]
#define OD_ENTRY_H6076_motorRatedTorque                &OD->list[65]
#define OD_ENTRY_H6077_torqueActualValue               &OD->list[66]
#define OD_ENTRY_H6079_DClinkCircuitVoltage            &OD->list[67]
#define OD_ENTRY_H607A_targetPosition                  &OD->list[68]
#define OD_ENTRY_H607D_softwarePositionLimit           &OD->list[69]
#define OD_ENTRY_H6080_maxMotorSpeed                   &OD->list[70]
#define OD_ENTRY_H6081_profileVelocity                 &OD->list[71]
#define OD_ENTRY_H6083_profileAcceleration             &OD->list[72]
#define OD_ENTRY_H6084_profileDeceleration             &OD->list[73]
#define OD_ENTRY_H6085_quickStopDeceleration           &OD->list[74]
#define OD_ENTRY_H60C5_maxAcceleration                 &OD->list[75]
#define OD_ENTRY_H60C6_maxDeceleration                 &OD->list[76]
#define OD_ENTRY_H60FF_targetVelocity                  &OD->list[77]
#define OD_ENTRY_H6502_supportedDriveModes             &OD->list[78]

#endif /* OD_H */
