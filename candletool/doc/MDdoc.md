CANopen device documentation
============================
**MD**

MD motor controller

|              |                                |
| ------------ | ------------------------------ |
| Project File | MDv1.0.0.eds                   |
| File Version | 1.0                            |
| Created      | 11/23/2020 1:00:00 PM          |
| Created By   | Piotr Wasilewski               |
| Modified     | 6/11/2024 11:08:00 AM          |
| Modified By  | Pawel Korsak                   |

This file was automatically generated with [libedssharp](https://github.com/robincornelius/libedssharp) Object Dictionary Editor v0.8-125-g85dfd8e

[TOC]


Device Information
------------------
|              |                                |
| ------------ | ------------------------------ |
| Vendor Name  | MAB ROBOTICS                   |
| Vendor ID    | 1                              |
| Product Name | MD                             |
| Product ID   | 1                              |
| Granularity  | 8                              |
| RPDO count   | 4                              |
| TPDO count   | 4                              |
| LSS Slave    | True                           |
| LSS Master   | False                          |

#### Supported Baud rates
* [x] 10 kBit/s
* [x] 20 kBit/s
* [x] 50 kBit/s
* [x] 125 kBit/s
* [x] 250 kBit/s
* [x] 500 kBit/s
* [x] 800 kBit/s
* [x] 1000 kBit/s
* [ ] auto


PDO Mapping
-----------

### RPDO 0x1400
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x200                                                         |
| Transmission | type=254                                                      |
|   0x60FF0020 | Target Velocity                                               |


### RPDO 0x1401
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x300                                                         |
| Transmission | type=254                                                      |
|   0x60400010 | Controlword                                                   |
|   0x60600008 | Modes Of Operation                                            |


### RPDO 0x1402
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x400                                                         |
| Transmission | type=254                                                      |
|   0x60400010 | Controlword                                                   |
|   0x607A0020 | Target Position                                               |


### RPDO 0x1403
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x500                                                         |
| Transmission | type=254                                                      |
|   0x60400010 | Controlword                                                   |
|   0x60FF0020 | Target Velocity                                               |


### TPDO 0x1800
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x180                                                         |
| Transmission | type=254; inhibit-time=0; event-timer=0; SYNC-start-value=0   |
|   0x60410010 | Statusword                                                    |


### TPDO 0x1801
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x280                                                         |
| Transmission | type=254; inhibit-time=0; event-timer=0; SYNC-start-value=0   |
|   0x60410010 | Statusword                                                    |
|   0x60640020 | Position Value                                                |


### TPDO 0x1802
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x380                                                         |
| Transmission | type=254; inhibit-time=0; event-timer=0; SYNC-start-value=0   |
|   0x60410010 | Statusword                                                    |
|   0x606C0020 | Velocity Actual Value                                         |


### TPDO 0x1803
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x480                                                         |
| Transmission | type=254; inhibit-time=0; event-timer=0; SYNC-start-value=0   |
|   0x60410010 | Statusword                                                    |
|   0x60770010 | Torque Actual Value                                           |


Communication Specific Parameters
---------------------------------

### 0x1000 - Device type
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | ro  | no  | no   | 0x00000192                      |

### 0x1001 - Error register
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED8               | ro  | tr  | no   | 0x00                            |

### 0x1003 - Pre-defined error field
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of errors      | UNSIGNED8  | rw  | no  | no   |               |
| 0x01 | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x02 | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x03 | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x04 | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x05 | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x06 | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x07 | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x08 | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x09 | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x0A | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x0B | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x0C | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x0D | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x0E | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x0F | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |
| 0x10 | Standard error field  | UNSIGNED32 | ro  | no  | no   |               |

### 0x1005 - COB-ID SYNC message
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0x00000080                      |

### 0x1006 - Communication cycle period
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0                               |

### 0x1007 - Synchronous window length
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0                               |

### 0x1008 - Manufacturer device name
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| VISIBLE_STRING          | ro  | no  | no   | MD80                            |

### 0x1009 - Manufacturer hardware version
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| VISIBLE_STRING          | ro  | no  | no   | 0                               |

### 0x1010 - Store parameters
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x04          |
| 0x01 | Save all parameters   | UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x02 | Save communication parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x03 | Save application parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x04 | Save manufacturer defined parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |

### 0x1011 - Restore default parameters
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x04          |
| 0x01 | Restore all default parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x02 | Restore communication default parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x03 | Restore application default parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x04 | Restore manufacturer defined default parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |

### 0x1012 - COB-ID time stamp object
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0x00000100                      |

### 0x1014 - COB-ID EMCY
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0x80+$NODEID                    |

### 0x1015 - Inhibit time EMCY
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED16              | rw  | no  | no   | 0                               |

### 0x1016 - Consumer heartbeat time
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x08          |
| 0x01 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x02 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x03 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x04 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x05 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x06 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x07 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x08 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |

### 0x1017 - Producer heartbeat time
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED16              | rw  | no  | no   | 1000                            |

### 0x1018 - Identity
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x04          |
| 0x01 | Vendor-ID             | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x02 | Product code          | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x03 | Revision number       | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x04 | Serial number         | UNSIGNED32 | ro  | no  | no   | 0x00000000    |

### 0x1019 - Synchronous counter overflow value
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED8               | rw  | no  | no   | 0                               |

### 0x1200 - SDO server parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 2             |
| 0x01 | COB-ID client to server (rx)| UNSIGNED32 | ro  | tr  | no   | 0x600+$NODEID |
| 0x02 | COB-ID server to client (tx)| UNSIGNED32 | ro  | tr  | no   | 0x580+$NODEID |

### 0x1201 - SDO server parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 2             |
| 0x01 | COB-ID client to server (rx)| UNSIGNED32 | ro  | tr  | no   | 0x600+$NODEID |
| 0x02 | COB-ID server to client (tx)| UNSIGNED32 | ro  | tr  | no   | 0x580+$NODEID |

### 0x1280 - SDO client parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x03          |
| 0x01 | COB-ID client to server (tx)| UNSIGNED32 | rw  | tr  | no   | 0x80000000    |
| 0x02 | COB-ID server to client (rx)| UNSIGNED32 | rw  | tr  | no   | 0x80000000    |
| 0x03 | Node-ID of the SDO server| UNSIGNED8  | rw  | no  | no   | 0x01          |

### 0x1400 - RPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | max sub-index         | UNSIGNED8  | ro  | no  | no   | 6             |
| 0x01 | COB-ID used by RPDO   | UNSIGNED32 | rw  | no  | no   | 0x200         |
| 0x02 | transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |

### 0x1401 - RPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | max sub-index         | UNSIGNED8  | ro  | no  | no   | 6             |
| 0x01 | COB-ID used by RPDO   | UNSIGNED32 | rw  | no  | no   | 0x300         |
| 0x02 | transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |

### 0x1402 - RPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | max sub-index         | UNSIGNED8  | ro  | no  | no   | 6             |
| 0x01 | COB-ID used by RPDO   | UNSIGNED32 | rw  | no  | no   | 0x400         |
| 0x02 | transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |

### 0x1403 - RPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | max sub-index         | UNSIGNED8  | ro  | no  | no   | 6             |
| 0x01 | COB-ID used by RPDO   | UNSIGNED32 | rw  | no  | no   | 0x500         |
| 0x02 | transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |

### 0x1600 - RPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped objects| UNSIGNED8  | rw  | no  | no   | 0x01          |
| 0x01 | Mapped object 1       | UNSIGNED32 | rw  | no  | no   | 0x60FF0020    |
| 0x02 | Mapped object 2       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x03 | Mapped object 3       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x04 | Mapped object 4       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x05 | Mapped object 5       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x06 | Mapped object 6       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x07 | Mapped object 7       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x08 | Mapped object 8       | UNSIGNED32 | rw  | no  | no   | 0x0           |

### 0x1601 - RPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped objects| UNSIGNED8  | rw  | no  | no   | 0x02          |
| 0x01 | Mapped object 1       | UNSIGNED32 | rw  | no  | no   | 0x60400010    |
| 0x02 | Mapped object 2       | UNSIGNED32 | rw  | no  | no   | 0x60600008    |
| 0x03 | Mapped object 3       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x04 | Mapped object 4       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x05 | Mapped object 5       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x06 | Mapped object 6       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x07 | Mapped object 7       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x08 | Mapped object 8       | UNSIGNED32 | rw  | no  | no   | 0x0           |

### 0x1602 - RPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped objects| UNSIGNED8  | rw  | no  | no   | 0x02          |
| 0x01 | Mapped object 1       | UNSIGNED32 | rw  | no  | no   | 0x60400010    |
| 0x02 | Mapped object 2       | UNSIGNED32 | rw  | no  | no   | 0x607a0020    |
| 0x03 | Mapped object 3       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x04 | Mapped object 4       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x05 | Mapped object 5       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x06 | Mapped object 6       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x07 | Mapped object 7       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x08 | Mapped object 8       | UNSIGNED32 | rw  | no  | no   | 0x0           |

### 0x1603 - RPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped objects| UNSIGNED8  | rw  | no  | no   | 0x02          |
| 0x01 | Mapped object 1       | UNSIGNED32 | rw  | no  | no   | 0x60400010    |
| 0x02 | Mapped object 2       | UNSIGNED32 | rw  | no  | no   | 0x60ff0020    |
| 0x03 | Mapped object 3       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x04 | Mapped object 4       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x05 | Mapped object 5       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x06 | Mapped object 6       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x07 | Mapped object 7       | UNSIGNED32 | rw  | no  | no   | 0x0           |
| 0x08 | Mapped object 8       | UNSIGNED32 | rw  | no  | no   | 0x0           |

### 0x1800 - TPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | max sub-index         | UNSIGNED8  | ro  | no  | no   | 6             |
| 0x01 | COB-ID used by TPDO   | UNSIGNED32 | rw  | no  | no   | 0x180         |
| 0x02 | transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |
| 0x03 | inhibit time          | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x04 | compatibility entry   | UNSIGNED8  | rw  | no  | no   | 0             |
| 0x05 | event timer           | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x06 | SYNC start value      | UNSIGNED8  | rw  | no  | no   | 0             |

### 0x1801 - TPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | max sub-index         | UNSIGNED8  | ro  | no  | no   | 6             |
| 0x01 | COB-ID used by TPDO   | UNSIGNED32 | rw  | no  | no   | 0x280         |
| 0x02 | transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |
| 0x03 | inhibit time          | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x04 | compatibility entry   | UNSIGNED8  | rw  | no  | no   | 0             |
| 0x05 | event timer           | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x06 | SYNC start value      | UNSIGNED8  | rw  | no  | no   | 0             |

### 0x1802 - TPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | max sub-index         | UNSIGNED8  | ro  | no  | no   | 6             |
| 0x01 | COB-ID used by TPDO   | UNSIGNED32 | rw  | no  | no   | 0x380         |
| 0x02 | transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |
| 0x03 | inhibit time          | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x04 | compatibility entry   | UNSIGNED8  | rw  | no  | no   | 0             |
| 0x05 | event timer           | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x06 | SYNC start value      | UNSIGNED8  | rw  | no  | no   | 0             |

### 0x1803 - TPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | max sub-index         | UNSIGNED8  | ro  | no  | no   | 6             |
| 0x01 | COB-ID used by TPDO   | UNSIGNED32 | rw  | no  | no   | 0x480         |
| 0x02 | transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |
| 0x03 | inhibit time          | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x04 | compatibility entry   | UNSIGNED8  | rw  | no  | no   | 0             |
| 0x05 | event timer           | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x06 | SYNC start value      | UNSIGNED8  | rw  | no  | no   | 0             |

### 0x1A00 - TPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped objects| UNSIGNED8  | rw  | no  | no   | 0x01          |
| 0x01 | Mapped object 1       | UNSIGNED32 | rw  | no  | no   | 0x60410010    |
| 0x02 | Mapped object 2       | UNSIGNED32 | rw  | no  | no   | 0x0           |

### 0x1A01 - TPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped objects| UNSIGNED8  | rw  | no  | no   | 0x02          |
| 0x01 | Mapped object 1       | UNSIGNED32 | rw  | no  | no   | 0x60410010    |
| 0x02 | Mapped object 2       | UNSIGNED32 | rw  | no  | no   | 0x60640020    |

### 0x1A02 - TPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped objects| UNSIGNED8  | rw  | no  | no   | 2             |
| 0x01 | Mapped object 1       | UNSIGNED32 | rw  | no  | no   | 0x60410010    |
| 0x02 | Mapped object 2       | UNSIGNED32 | rw  | no  | no   | 0x606c0020    |

### 0x1A03 - TPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped objects| UNSIGNED8  | rw  | no  | no   | 2             |
| 0x01 | Mapped object 1       | UNSIGNED32 | rw  | no  | no   | 0x60410010    |
| 0x02 | Mapped object 2       | UNSIGNED32 | rw  | no  | no   | 0x60770010    |

Manufacturer Specific Parameters
--------------------------------

### 0x2000 - Motor Settings
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x0D          |
| 0x01 | Pole pairs            | UNSIGNED32 | rw  | no  | no   | 0             |
| 0x02 | Torque constant       | REAL32     | rw  | no  | no   | 0             |
| 0x03 | Phase Inductance      | REAL32     | ro  | no  | no   | 0             |
| 0x04 | Phase resistance      | REAL32     | ro  | no  | no   | 0             |
| 0x05 | Torque Bandwidth      | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x06 | Motor Name            | VISIBLE_STRING| rw  | no  | no   | MD80          |
| 0x07 | Motor Shutdown Temperature| UNSIGNED8  | rw  | no  | no   | 80            |
| 0x08 | Gear Ratio            | REAL32     | rw  | no  | no   | 1             |
| 0x09 | Calibration Mode      | UNSIGNED8  | rw  | no  | no   | 0             |
| 0x0A | Can ID                | UNSIGNED32 | rw  | no  | no   | 1             |
| 0x0B | Can Baudrate          | UNSIGNED32 | rw  | no  | no   | 1             |
| 0x0C | Can Watchdog          | UNSIGNED16 | rw  | no  | no   | 1             |
| 0x0D | Reverse Direction     | BOOLEAN    | rw  | no  | no   | false         |

### 0x2001 - Velocity PID Controller
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x04          |
| 0x01 | Kp                    | REAL32     | rw  | tr  | no   | 0             |
| 0x02 | Ki                    | REAL32     | rw  | tr  | no   | 0             |
| 0x03 | Kd                    | REAL32     | rw  | tr  | no   | 0             |
| 0x04 | Integral Limit        | REAL32     | rw  | no  | no   | 0             |

### 0x2002 - Position PID Controller
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x04          |
| 0x01 | Kp                    | REAL32     | rw  | tr  | no   | 0             |
| 0x02 | Ki                    | REAL32     | rw  | tr  | no   | 0             |
| 0x03 | Kd                    | REAL32     | rw  | tr  | no   | 0             |
| 0x04 | Integral Limit        | REAL32     | rw  | no  | no   | 0             |

### 0x2003 - System Command
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x0D          |
| 0x01 | Blink LEDs            | BOOLEAN    | rw  | no  | no   | 0             |
| 0x02 | Reset Controller      | BOOLEAN    | rw  | no  | no   | 0             |
| 0x03 | Run Calibration       | BOOLEAN    | rw  | no  | no   | 0             |
| 0x04 | Run Output Encoder Calibration| BOOLEAN    | rw  | no  | no   | 0             |
| 0x05 | Set Zero              | BOOLEAN    | rw  | no  | no   | 0             |
| 0x06 | Calibrate Current PI Gains| BOOLEAN    | rw  | no  | no   | 0             |
| 0x07 | Test Output Encoder   | BOOLEAN    | rw  | no  | no   | 0             |
| 0x08 | Test Main Encoder     | BOOLEAN    | rw  | no  | no   | 0             |
| 0x09 | Save Memory           | BOOLEAN    | rw  | no  | no   | 0             |
| 0x0A | Revert Factory Settings| BOOLEAN    | rw  | no  | no   | 0             |
| 0x0B | Clear Errors          | BOOLEAN    | rw  | no  | no   | 0             |
| 0x0C | Clear Warnings        | BOOLEAN    | rw  | no  | no   | 0             |
| 0x0D | Run Can Reinit        | BOOLEAN    | rw  | no  | no   | 0             |

### 0x2004 - System Status
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x08          |
| 0x01 | Main Encoder Status   | UNSIGNED32 | rw  | no  | no   | 0             |
| 0x02 | Output Encoder Status | UNSIGNED32 | rw  | no  | no   | 0             |
| 0x03 | Calibration Status    | UNSIGNED32 | rw  | no  | no   | 0             |
| 0x04 | Bridge Status         | UNSIGNED32 | rw  | no  | no   | 0             |
| 0x05 | Hardware Status       | UNSIGNED32 | rw  | no  | no   | 0             |
| 0x06 | Homing Status         | UNSIGNED32 | rw  | no  | no   | 0             |
| 0x07 | Motion Status         | UNSIGNED32 | rw  | no  | no   | 0             |
| 0x08 | Communication Status  | UNSIGNED32 | rw  | no  | no   | 0             |

### 0x2005 - Output Encoder
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x05          |
| 0x01 | Type                  | UNSIGNED8  | rw  | no  | no   | 0             |
| 0x02 | Calibration Mode      | UNSIGNED8  | rw  | no  | no   | 0             |
| 0x03 | Mode                  | UNSIGNED8  | rw  | no  | no   | 0             |
| 0x04 | Position              | REAL32     | ro  | tr  | no   | 0             |
| 0x05 | Velocity              | REAL32     | rw  | r   | no   | 0             |

### 0x2006 - Temperature
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x02          |
| 0x01 | Motor Temperature     | REAL32     | ro  | tr  | no   | 0             |
| 0x02 | Mosfet Temperature    | REAL32     | ro  | tr  | no   | 0             |

### 0x2007 - Limits
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x06          |
| 0x01 | Max Torque            | REAL32     | rw  | no  | no   | 0             |
| 0x02 | Max Acceleration      | REAL32     | rw  | no  | no   | 0             |
| 0x03 | Max Deceleration      | REAL32     | rw  | no  | no   | 0             |
| 0x04 | Max Velocity          | REAL32     | rw  | no  | no   | 0             |
| 0x05 | Max Position          | REAL32     | rw  | no  | no   | 0             |
| 0x06 | Min Position          | REAL32     | rw  | no  | no   | 0             |

### 0x2008 - Motion
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x0B          |
| 0x01 | Mode Command          | UNSIGNED8  | rw  | t   | no   | 0             |
| 0x02 | Mode Status           | UNSIGNED8  | rw  | t   | no   | 0             |
| 0x03 | Profile Velocity      | REAL32     | rw  | t   | no   | 0             |
| 0x04 | Profile Acceleration  | REAL32     | rw  | t   | no   | 0             |
| 0x05 | Profile Deceleration  | REAL32     | rw  | t   | no   | 0             |
| 0x06 | Quick Stop Deceleration| REAL32     | rw  | t   | no   | 0             |
| 0x07 | Position Window       | REAL32     | rw  | t   | no   | 0             |
| 0x08 | Velocity Window       | REAL32     | rw  | t   | no   | 0             |
| 0x09 | Target Position       | REAL32     | rw  | tr  | no   | 0             |
| 0x0A | Target Velocity       | REAL32     | rw  | tr  | no   | 0             |
| 0x0B | Target Torque         | REAL32     | rw  | tr  | no   | 0             |

### 0x2009 - Motor Mesurements
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x03          |
| 0x01 | Output Position       | REAL32     | rw  | r   | no   | 0             |
| 0x02 | Output Velocity       | REAL32     | rw  | r   | no   | 0             |
| 0x03 | Output Torque         | REAL32     | rw  | r   | no   | 0             |

### 0x200A - Firmware info
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x03          |
| 0x01 | Commit Hash           | VISIBLE_STRING| ro  | no  | no   | 0             |
| 0x02 | Build Date            | UNSIGNED32 | ro  | no  | no   | 0             |
| 0x03 | Version               | UNSIGNED32 | ro  | no  | no   | 0             |

### 0x200B - Bootloder Info
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x04          |
| 0x01 | Bootloader Fixed      | BOOLEAN    | ro  | no  | no   | 0             |
| 0x02 | Commit Hash           | VISIBLE_STRING| ro  | no  | no   | 0             |
| 0x03 | Build Date            | UNSIGNED32 | ro  | no  | no   | 0             |
| 0x04 | Version               | UNSIGNED32 | ro  | no  | no   | 0             |

### 0x200C - Impedance PD Controller
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x02          |
| 0x01 | Kp                    | REAL32     | rw  | tr  | no   | 0             |
| 0x02 | Kd                    | REAL32     | rw  | tr  | no   | 0             |

### 0x200D - User GPIO
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x02          |
| 0x01 | User GPIO Configuration| UNSIGNED8  | rw  | no  | no   | 0             |
| 0x02 | User GPIO State       | UNSIGNED16 | ro  | no  | no   | 0             |

### 0x200E - DC bus voltage
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| REAL32                  | ro  | no  | no   | 0                               |

Device Profile Specific Parameters
----------------------------------

### 0x6040 - Controlword
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED16              | rw  | tr  | no   | 0                               |

### 0x6041 - Statusword
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED16              | ro  | tr  | no   | 0                               |

### 0x6060 - Modes Of Operation
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| INTEGER8                | rw  | r   | no   | 0                               |

### 0x6061 - Modes Of Operation Display
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| INTEGER8                | ro  | tr  | no   | 0                               |

### 0x6062 - Position Demand Value
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| INTEGER32               | ro  | no  | no   | 0                               |

### 0x6064 - Position Value
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| INTEGER32               | ro  | tr  | no   | 0                               |

### 0x6067 - Position Window
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0                               |

### 0x606B - Velocity Demand Value
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| INTEGER32               | rw  | no  | no   | 0                               |

### 0x606C - Velocity Actual Value
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| INTEGER32               | ro  | tr  | no   | 0                               |

### 0x606D - Velocity Window
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0                               |

### 0x6072 - Max Torque
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED16              | rw  | tr  | no   | 0                               |

### 0x6073 - Max Current
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED16              | rw  | tr  | no   | 0                               |

### 0x6074 - Torque Demand Value
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| INTEGER16               | rw  | no  | no   | 0                               |

### 0x6075 - Motor Rated Current
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0                               |

### 0x6076 - Motor Rated Torque
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0                               |

### 0x6077 - Torque Actual Value
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| INTEGER16               | ro  | tr  | no   | 0                               |

### 0x6079 - DClink Circuit Voltage
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | ro  | tr  | no   | 0                               |

### 0x607A - Target Position
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| INTEGER32               | rw  | tr  | no   | 0                               |

### 0x607D - Software Position Limit
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x02          |
| 0x01 | Min position limit    | INTEGER32  | rw  | tr  | no   | -2147483648   |
| 0x02 | Max position limit    | INTEGER32  | rw  | tr  | no   | 2147483647    |

### 0x6080 - Max Motor Speed
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 1000                            |

### 0x6081 - Profile Velocity
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | r   | no   | 0                               |

### 0x6083 - Profile Acceleration
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | tr  | no   | 0                               |

### 0x6084 - Profile Deceleration
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | tr  | no   | 0                               |

### 0x6085 - Quick Stop Deceleration
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | r   | no   | 10000                           |

### 0x60C5 - Max Acceleration
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | tr  | no   | 0                               |

### 0x60C6 - Max Deceleration
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | tr  | no   | 0                               |

### 0x60FF - Target Velocity
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| INTEGER32               | rw  | tr  | no   | 0                               |

### 0x6502 - Supported Drive Modes
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | ro  | no  | no   | 647                             |
