#include "OD.hpp"

using namespace mab;

std::vector<edsObject> mab::generateObjectDictionary()
{
    std::vector<edsObject> list;
    list.push_back(
        edsObject{0x1000, 0x0, "Device type", 0x7, "PERSIST_COMM", 0x7, "ro", false, 0x192});
    list.push_back(edsObject{0x1001, 0x0, "Error register", 0x7, "RAM", 0x5, "ro", true, 0x0});
    list.push_back(
        edsObject{0x1003, 0x0, "Pre-defined error field", 0x8, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x1003, 0x0, "Number of errors", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0x1, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0x2, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0x3, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0x4, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0x5, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0x6, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0x7, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0x8, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0x9, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0xa, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0xb, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0xc, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0xd, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0xe, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0xf, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1003, 0x10, "Standard error field", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1005, 0x0, "COB-ID SYNC message", 0x7, "PERSIST_COMM", 0x7, "rw", false, 0x80});
    list.push_back(edsObject{
        0x1006, 0x0, "Communication cycle period", 0x7, "PERSIST_COMM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1007, 0x0, "Synchronous window length", 0x7, "PERSIST_COMM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1008, 0x0, "Manufacturer device name", 0x7, "PERSIST_COMM", 0x9, "ro", false, 0x0});
    list.push_back(edsObject{
        0x1009, 0x0, "Manufacturer hardware version", 0x7, "PERSIST_COMM", 0x9, "ro", false, 0x0});
    list.push_back(edsObject{0x1010, 0x0, "Store parameters", 0x8, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1010, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x4});
    list.push_back(
        edsObject{0x1010, 0x1, "Save all parameters", 0x7, "RAM", 0x7, "rw", false, 0x1});
    list.push_back(
        edsObject{0x1010, 0x2, "Save communication parameters", 0x7, "RAM", 0x7, "rw", false, 0x1});
    list.push_back(
        edsObject{0x1010, 0x3, "Save application parameters", 0x7, "RAM", 0x7, "rw", false, 0x1});
    list.push_back(edsObject{
        0x1010, 0x4, "Save manufacturer defined parameters", 0x7, "RAM", 0x7, "rw", false, 0x1});
    list.push_back(
        edsObject{0x1011, 0x0, "Restore default parameters", 0x8, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1011, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x4});
    list.push_back(edsObject{
        0x1011, 0x1, "Restore all default parameters", 0x7, "RAM", 0x7, "rw", false, 0x1});
    list.push_back(edsObject{0x1011,
                             0x2,
                             "Restore communication default parameters",
                             0x7,
                             "RAM",
                             0x7,
                             "rw",
                             false,
                             0x1});
    list.push_back(edsObject{
        0x1011, 0x3, "Restore application default parameters", 0x7, "RAM", 0x7, "rw", false, 0x1});
    list.push_back(edsObject{0x1011,
                             0x4,
                             "Restore manufacturer defined default parameters",
                             0x7,
                             "RAM",
                             0x7,
                             "rw",
                             false,
                             0x1});
    list.push_back(edsObject{
        0x1012, 0x0, "COB-ID time stamp object", 0x7, "PERSIST_COMM", 0x7, "rw", false, 0x100});
    list.push_back(
        edsObject{0x1014, 0x0, "COB-ID EMCY", 0x7, "PERSIST_COMM", 0x7, "rw", false, 0x80});
    list.push_back(
        edsObject{0x1015, 0x0, "Inhibit time EMCY", 0x7, "PERSIST_COMM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1016, 0x0, "Consumer heartbeat time", 0x8, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1016, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x8});
    list.push_back(
        edsObject{0x1016, 0x1, "Consumer heartbeat time", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1016, 0x2, "Consumer heartbeat time", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1016, 0x3, "Consumer heartbeat time", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1016, 0x4, "Consumer heartbeat time", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1016, 0x5, "Consumer heartbeat time", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1016, 0x6, "Consumer heartbeat time", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1016, 0x7, "Consumer heartbeat time", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1016, 0x8, "Consumer heartbeat time", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1017, 0x0, "Producer heartbeat time", 0x7, "PERSIST_COMM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x1018, 0x0, "Identity", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1018, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x4});
    list.push_back(edsObject{0x1018, 0x1, "Vendor-ID", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x1018, 0x2, "Product code", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x1018, 0x3, "Revision number", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x1018, 0x4, "Serial number", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x1019,
                             0x0,
                             "Synchronous counter overflow value",
                             0x7,
                             "PERSIST_COMM",
                             0x5,
                             "rw",
                             false,
                             0x0});
    list.push_back(
        edsObject{0x1200, 0x0, "SDO server parameter", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1200, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1200, 0x1, "COB-ID client to server (rx)", 0x7, "RAM", 0x7, "ro", true, 0x600});
    list.push_back(
        edsObject{0x1200, 0x2, "COB-ID server to client (tx)", 0x7, "RAM", 0x7, "ro", true, 0x580});
    list.push_back(
        edsObject{0x1201, 0x0, "SDO server parameter", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1201, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1201, 0x1, "COB-ID client to server (rx)", 0x7, "RAM", 0x7, "ro", true, 0x600});
    list.push_back(
        edsObject{0x1201, 0x2, "COB-ID server to client (tx)", 0x7, "RAM", 0x7, "ro", true, 0x580});
    list.push_back(
        edsObject{0x1280, 0x0, "SDO client parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1280, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x3});
    list.push_back(edsObject{
        0x1280, 0x1, "COB-ID client to server (tx)", 0x7, "RAM", 0x7, "rw", true, 0x80000000});
    list.push_back(edsObject{
        0x1280, 0x2, "COB-ID server to client (rx)", 0x7, "RAM", 0x7, "rw", true, 0x80000000});
    list.push_back(
        edsObject{0x1280, 0x3, "Node-ID of the SDO server", 0x7, "RAM", 0x5, "rw", false, 0x1});
    list.push_back(edsObject{
        0x1400, 0x0, "RPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x1400, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1400, 0x1, "COB-ID used by RPDO", 0x7, "RAM", 0x7, "rw", false, 0x200});
    list.push_back(edsObject{0x1400, 0x2, "transmission type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1401, 0x0, "RPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x1401, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1401, 0x1, "COB-ID used by RPDO", 0x7, "RAM", 0x7, "rw", false, 0x300});
    list.push_back(edsObject{0x1401, 0x2, "transmission type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x1402, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(edsObject{
        0x1402, 0x0, "RPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1402, 0x1, "COB-ID used by RPDO", 0x7, "RAM", 0x7, "rw", false, 0x400});
    list.push_back(edsObject{0x1402, 0x2, "transmission type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1403, 0x0, "RPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x1403, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1403, 0x1, "COB-ID used by RPDO", 0x7, "RAM", 0x7, "rw", false, 0x500});
    list.push_back(edsObject{0x1403, 0x2, "transmission type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1600, 0x0, "RPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1600, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x1});
    list.push_back(
        edsObject{0x1600, 0x1, "Mapped object 1", 0x7, "RAM", 0x7, "rw", false, 0x60ff0020});
    list.push_back(edsObject{0x1600, 0x2, "Mapped object 2", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1600, 0x3, "Mapped object 3", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1600, 0x4, "Mapped object 4", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1600, 0x5, "Mapped object 5", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1600, 0x6, "Mapped object 6", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1600, 0x7, "Mapped object 7", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1600, 0x8, "Mapped object 8", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1601, 0x0, "RPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1601, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x2});
    list.push_back(
        edsObject{0x1601, 0x1, "Mapped object 1", 0x7, "RAM", 0x7, "rw", false, 0x60400010});
    list.push_back(
        edsObject{0x1601, 0x2, "Mapped object 2", 0x7, "RAM", 0x7, "rw", false, 0x60600008});
    list.push_back(edsObject{0x1601, 0x3, "Mapped object 3", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1601, 0x4, "Mapped object 4", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1601, 0x5, "Mapped object 5", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1601, 0x6, "Mapped object 6", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1601, 0x7, "Mapped object 7", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1601, 0x8, "Mapped object 8", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1602, 0x0, "RPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1602, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x2});
    list.push_back(
        edsObject{0x1602, 0x1, "Mapped object 1", 0x7, "RAM", 0x7, "rw", false, 0x60400010});
    list.push_back(
        edsObject{0x1602, 0x2, "Mapped object 2", 0x7, "RAM", 0x7, "rw", false, 0x607a0020});
    list.push_back(edsObject{0x1602, 0x3, "Mapped object 3", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1602, 0x4, "Mapped object 4", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1602, 0x5, "Mapped object 5", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1602, 0x6, "Mapped object 6", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1602, 0x7, "Mapped object 7", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1602, 0x8, "Mapped object 8", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1603, 0x0, "RPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1603, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x2});
    list.push_back(
        edsObject{0x1603, 0x1, "Mapped object 1", 0x7, "RAM", 0x7, "rw", false, 0x60400010});
    list.push_back(
        edsObject{0x1603, 0x2, "Mapped object 2", 0x7, "RAM", 0x7, "rw", false, 0x60ff0020});
    list.push_back(edsObject{0x1603, 0x3, "Mapped object 3", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1603, 0x4, "Mapped object 4", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1603, 0x5, "Mapped object 5", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1603, 0x6, "Mapped object 6", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1603, 0x7, "Mapped object 7", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x1603, 0x8, "Mapped object 8", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1800, 0x0, "TPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x1800, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1800, 0x1, "COB-ID used by TPDO", 0x7, "RAM", 0x7, "rw", false, 0x180});
    list.push_back(edsObject{0x1800, 0x2, "transmission type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x1800, 0x3, "inhibit time", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1800, 0x4, "compatibility entry", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x1800, 0x5, "event timer", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x1800, 0x6, "SYNC start value", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1801, 0x0, "TPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x1801, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1801, 0x1, "COB-ID used by TPDO", 0x7, "RAM", 0x7, "rw", false, 0x280});
    list.push_back(edsObject{0x1801, 0x2, "transmission type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x1801, 0x3, "inhibit time", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1801, 0x4, "compatibility entry", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x1801, 0x5, "event timer", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x1801, 0x6, "SYNC start value", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1802, 0x0, "TPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x1802, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1802, 0x1, "COB-ID used by TPDO", 0x7, "RAM", 0x7, "rw", false, 0x380});
    list.push_back(edsObject{0x1802, 0x2, "transmission type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x1802, 0x3, "inhibit time", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1802, 0x4, "compatibility entry", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x1802, 0x5, "event timer", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x1802, 0x6, "SYNC start value", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1803, 0x0, "TPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x1803, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1803, 0x1, "COB-ID used by TPDO", 0x7, "RAM", 0x7, "rw", false, 0x480});
    list.push_back(edsObject{0x1803, 0x2, "transmission type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x1803, 0x3, "inhibit time", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1803, 0x4, "compatibility entry", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x1803, 0x5, "event timer", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x1803, 0x6, "SYNC start value", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1a00, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x1});
    list.push_back(edsObject{
        0x1a00, 0x0, "TPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1a00, 0x1, "Mapped object 1", 0x7, "RAM", 0x7, "rw", false, 0x60410010});
    list.push_back(edsObject{0x1a00, 0x2, "Mapped object 2", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1a01, 0x0, "TPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1a01, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x2});
    list.push_back(
        edsObject{0x1a01, 0x1, "Mapped object 1", 0x7, "RAM", 0x7, "rw", false, 0x60410010});
    list.push_back(
        edsObject{0x1a01, 0x2, "Mapped object 2", 0x7, "RAM", 0x7, "rw", false, 0x60640020});
    list.push_back(
        edsObject{0x1a02, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1a02, 0x0, "TPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1a02, 0x1, "Mapped object 1", 0x7, "RAM", 0x7, "rw", false, 0x60410010});
    list.push_back(
        edsObject{0x1a02, 0x2, "Mapped object 2", 0x7, "RAM", 0x7, "rw", false, 0x606c0020});
    list.push_back(edsObject{
        0x1a03, 0x0, "TPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1a03, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1a03, 0x1, "Mapped object 1", 0x7, "RAM", 0x7, "rw", false, 0x60410010});
    list.push_back(
        edsObject{0x1a03, 0x2, "Mapped object 2", 0x7, "RAM", 0x7, "rw", false, 0x60770010});
    list.push_back(edsObject{0x2000, 0x0, "Motor Settings", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2000, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0xd});
    list.push_back(edsObject{0x2000, 0x1, "Pole pairs", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2000, 0x2, "Torque constant", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2000, 0x3, "Phase Inductance", 0x7, "RAM", 0x8, "ro", false, 0x0});
    list.push_back(edsObject{0x2000, 0x4, "Phase resistance", 0x7, "RAM", 0x8, "ro", false, 0x0});
    list.push_back(edsObject{0x2000, 0x5, "Torque Bandwidth", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x2000, 0x6, "Motor Name", 0x7, "RAM", 0x9, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2000, 0x7, "Motor Shutdown Temperature", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2000, 0x8, "Gear Ratio", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2000, 0x9, "Calibration Mode", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2000, 0xa, "Can ID", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2000, 0xb, "Can Baudrate", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2000, 0xc, "Can Watchdog", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x2000, 0xd, "Reverse Direction", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2001, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x4});
    list.push_back(
        edsObject{0x2001, 0x0, "Velocity PID Controller", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x2001, 0x1, "Kp_velocity", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(edsObject{0x2001, 0x2, "Ki_velocity", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(edsObject{0x2001, 0x3, "Kd_velocity", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(
        edsObject{0x2001, 0x4, "Integral Limit_velocity", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2002, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x4});
    list.push_back(
        edsObject{0x2002, 0x0, "Position PID Controller", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x2002, 0x1, "Kp_position", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(edsObject{0x2002, 0x2, "Ki_position", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(edsObject{0x2002, 0x3, "Kd_position", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(
        edsObject{0x2002, 0x4, "Integral Limit_position", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2003, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0xd});
    list.push_back(edsObject{0x2003, 0x0, "System Command", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x2003, 0x1, "Blink LEDs", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(edsObject{0x2003, 0x2, "Reset Controller", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(edsObject{0x2003, 0x3, "Run Calibration", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(edsObject{
        0x2003, 0x4, "Run Output Encoder Calibration", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(edsObject{0x2003, 0x5, "Set Zero", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2003, 0x6, "Calibrate Current PI Gains", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2003, 0x7, "Test Output Encoder", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(edsObject{0x2003, 0x8, "Test Main Encoder", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(edsObject{0x2003, 0x9, "Save Memory", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2003, 0xa, "Revert Factory Settings", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(edsObject{0x2003, 0xb, "Clear Errors", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(edsObject{0x2003, 0xc, "Clear Warnings", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(edsObject{0x2003, 0xd, "Run Can Reinit", 0x7, "RAM", 0x1, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2004, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x8});
    list.push_back(edsObject{0x2004, 0x0, "System Status", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2004, 0x1, "Main Encoder Status", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2004, 0x2, "Output Encoder Status", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2004, 0x3, "Calibration Status", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2004, 0x4, "Bridge Status", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2004, 0x5, "Hardware Status", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2004, 0x6, "Homing Status", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2004, 0x7, "Motion Status", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2004, 0x8, "Communication Status", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2005, 0x0, "Output Encoder", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2005, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x5});
    list.push_back(edsObject{0x2005, 0x1, "Type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2005, 0x2, "Calibration Mode", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2005, 0x3, "Mode", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2005, 0x4, "Position", 0x7, "RAM", 0x8, "ro", true, 0x0});
    list.push_back(edsObject{0x2005, 0x5, "Velocity", 0x7, "RAM", 0x8, "rwr", true, 0x0});
    list.push_back(edsObject{0x2006, 0x0, "Temperature", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2006, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x2});
    list.push_back(edsObject{0x2006, 0x1, "Motor Temperature", 0x7, "RAM", 0x8, "ro", true, 0x0});
    list.push_back(edsObject{0x2006, 0x2, "Mosfet Temperature", 0x7, "RAM", 0x8, "ro", true, 0x0});
    list.push_back(edsObject{0x2007, 0x0, "Limits", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2007, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x6});
    list.push_back(edsObject{0x2007, 0x1, "Max Torque", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2007, 0x2, "Max Acceleration", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2007, 0x3, "Max Deceleration", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2007, 0x4, "Max Velocity", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2007, 0x5, "Max Position", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2007, 0x6, "Min Position", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2008, 0x0, "Motion", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2008, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0xb});
    list.push_back(edsObject{0x2008, 0x1, "Mode Command", 0x7, "RAM", 0x5, "rww", true, 0x0});
    list.push_back(edsObject{0x2008, 0x2, "Mode Status", 0x7, "RAM", 0x5, "rww", true, 0x0});
    list.push_back(edsObject{0x2008, 0x3, "Profile Velocity", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(
        edsObject{0x2008, 0x4, "Profile Acceleration", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(
        edsObject{0x2008, 0x5, "Profile Deceleration", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(
        edsObject{0x2008, 0x6, "Quick Stop Deceleration", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x2008, 0x7, "Position Window", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x2008, 0x8, "Velocity Window", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x2008, 0x9, "Target Position", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(edsObject{0x2008, 0xa, "Target Velocity", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(edsObject{0x2008, 0xb, "Target Torque", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(edsObject{0x2009, 0x0, "Motor Mesurements", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2009, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x3});
    list.push_back(edsObject{0x2009, 0x1, "Output Position", 0x7, "RAM", 0x8, "rwr", true, 0x0});
    list.push_back(edsObject{0x2009, 0x2, "Output Velocity", 0x7, "RAM", 0x8, "rwr", true, 0x0});
    list.push_back(edsObject{0x2009, 0x3, "Output Torque", 0x7, "RAM", 0x8, "rwr", true, 0x0});
    list.push_back(edsObject{0x200a, 0x0, "Firmware info", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x200a, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x3});
    list.push_back(edsObject{0x200a, 0x1, "Commit Hash", 0x7, "RAM", 0x9, "ro", false, 0x0});
    list.push_back(edsObject{0x200a, 0x2, "Build Date", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x200a, 0x3, "Version", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x200b, 0x0, "Bootloder Info", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x200b, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x4});
    list.push_back(edsObject{0x200b, 0x1, "Bootloader Fixed", 0x7, "RAM", 0x1, "ro", false, 0x0});
    list.push_back(edsObject{0x200b, 0x2, "Commit Hash", 0x7, "RAM", 0x9, "ro", false, 0x0});
    list.push_back(edsObject{0x200b, 0x3, "Build Date", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x200b, 0x4, "Version", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x200c, 0x0, "Impedance PD Controller", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x200c, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x2});
    list.push_back(edsObject{0x200c, 0x1, "Kp_impedance", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(edsObject{0x200c, 0x2, "Kd_impedance", 0x7, "RAM", 0x8, "rw", true, 0x0});
    list.push_back(edsObject{0x200d, 0x0, "User GPIO", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x200d, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x2});
    list.push_back(
        edsObject{0x200d, 0x1, "User GPIO Configuration", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x200d, 0x2, "User GPIO State", 0x7, "RAM", 0x6, "ro", false, 0x0});
    list.push_back(edsObject{0x200e, 0x0, "DC bus voltage", 0x7, "RAM", 0x8, "ro", false, 0x0});
    list.push_back(edsObject{0x6040, 0x0, "Controlword", 0x7, "RAM", 0x6, "rw", true, 0x0});
    list.push_back(edsObject{0x6041, 0x0, "Statusword", 0x7, "RAM", 0x6, "ro", true, 0x0});
    list.push_back(edsObject{0x6060, 0x0, "Modes Of Operation", 0x7, "RAM", 0x2, "rwr", true, 0x0});
    list.push_back(
        edsObject{0x6061, 0x0, "Modes Of Operation Display", 0x7, "RAM", 0x2, "ro", true, 0x0});
    list.push_back(
        edsObject{0x6062, 0x0, "Position Demand Value", 0x7, "RAM", 0x4, "rw", false, 0x0});
    list.push_back(edsObject{0x6064, 0x0, "Position Value", 0x7, "RAM", 0x4, "ro", true, 0x0});
    list.push_back(edsObject{0x6067, 0x0, "Position Window", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(
        edsObject{0x606b, 0x0, "Velocity Demand Value", 0x7, "RAM", 0x4, "rw", false, 0x0});
    list.push_back(
        edsObject{0x606c, 0x0, "Velocity Actual Value", 0x7, "RAM", 0x4, "ro", true, 0x0});
    list.push_back(edsObject{0x606d, 0x0, "Velocity Window", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x6072, 0x0, "Motor Max Torque", 0x7, "RAM", 0x6, "rw", true, 0x0});
    list.push_back(edsObject{0x6073, 0x0, "Max Current", 0x7, "RAM", 0x6, "rw", true, 0x0});
    list.push_back(
        edsObject{0x6074, 0x0, "Torque Demand Value", 0x7, "RAM", 0x3, "rw", false, 0x0});
    list.push_back(
        edsObject{0x6075, 0x0, "Motor Rated Current", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x6076, 0x0, "Motor Rated Torque", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x6077, 0x0, "Torque Actual Value", 0x7, "RAM", 0x3, "ro", true, 0x0});
    list.push_back(
        edsObject{0x6079, 0x0, "DClink Circuit Voltage", 0x7, "RAM", 0x7, "ro", true, 0x0});
    list.push_back(
        edsObject{0x607a, 0x0, "Motor Target Position", 0x7, "RAM", 0x4, "rw", true, 0x0});
    list.push_back(
        edsObject{0x607d, 0x0, "Software Position Limit", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x607d, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x2});
    list.push_back(edsObject{0x607d, 0x1, "Min position limit", 0x7, "RAM", 0x4, "rw", true, 0x0});
    list.push_back(edsObject{0x607d, 0x2, "Max position limit", 0x7, "RAM", 0x4, "rw", true, 0x0});
    list.push_back(edsObject{0x6080, 0x0, "Max Motor Speed", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x6081, 0x0, "Profile Velocity", 0x7, "RAM", 0x7, "rwr", true, 0x0});
    list.push_back(
        edsObject{0x6083, 0x0, "Profile Acceleration", 0x7, "RAM", 0x7, "rw", true, 0x0});
    list.push_back(
        edsObject{0x6084, 0x0, "Profile Deceleration", 0x7, "RAM", 0x7, "rw", true, 0x0});
    list.push_back(
        edsObject{0x6085, 0x0, "Quick Stop Deceleration", 0x7, "RAM", 0x7, "rwr", true, 0x0});
    list.push_back(
        edsObject{0x60c5, 0x0, "Motor Max Acceleration", 0x7, "RAM", 0x7, "rw", true, 0x0});
    list.push_back(
        edsObject{0x60c6, 0x0, "Motor Max Deceleration", 0x7, "RAM", 0x7, "rw", true, 0x0});
    list.push_back(
        edsObject{0x60ff, 0x0, "Motor Target Velocity", 0x7, "RAM", 0x4, "rw", true, 0x0});
    list.push_back(
        edsObject{0x6502, 0x0, "Supported Drive Modes", 0x7, "RAM", 0x7, "ro", false, 0x0});
    return list;
}
