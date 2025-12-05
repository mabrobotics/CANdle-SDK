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
    list.push_back(edsObject{
        0x1402, 0x0, "RPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x1402, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1402, 0x1, "COB-ID used by RPDO", 0x7, "RAM", 0x7, "rw", false, 0x400});
    list.push_back(edsObject{0x1402, 0x2, "transmission type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{
        0x1403, 0x0, "RPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x1403, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(
        edsObject{0x1403, 0x1, "COB-ID used by RPDO", 0x7, "RAM", 0x7, "rw", false, 0x500});
    list.push_back(edsObject{0x1403, 0x2, "transmission type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(
        edsObject{0x1600, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x1});
    list.push_back(edsObject{
        0x1600, 0x0, "RPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
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
    list.push_back(
        edsObject{0x1602, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x2});
    list.push_back(edsObject{
        0x1602, 0x0, "RPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
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
    list.push_back(
        edsObject{0x1603, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x2});
    list.push_back(edsObject{
        0x1603, 0x0, "RPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
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
    list.push_back(edsObject{0x1800, 0x0, "max sub-index", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(edsObject{
        0x1800, 0x0, "TPDO communication parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
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
    list.push_back(edsObject{
        0x1a02, 0x0, "TPDO mapping parameter", 0x9, "PERSIST_COMM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x1a02, 0x0, "Number of mapped objects", 0x7, "RAM", 0x5, "rw", false, 0x0});
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
    list.push_back(edsObject{0x2000, 0x0, "Flash programmed", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x2001, 0x0, "Firmware info", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2001, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x3});
    list.push_back(edsObject{0x2001, 0x1, "Commit Hash", 0x7, "RAM", 0x9, "ro", false, 0x0});
    list.push_back(edsObject{0x2001, 0x2, "Build Date", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2001, 0x3, "Version", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2002, 0x0, "Bootloader info", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2002, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x4});
    list.push_back(edsObject{0x2002, 0x1, "Bootloader Fixed", 0x7, "RAM", 0x1, "ro", false, 0x0});
    list.push_back(edsObject{0x2002, 0x2, "Commit Hash", 0x7, "RAM", 0x9, "ro", false, 0x0});
    list.push_back(edsObject{0x2002, 0x3, "Build Date", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2002, 0x4, "Version", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x2003, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x5});
    list.push_back(edsObject{0x2003, 0x0, "Hardware info", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x2003, 0x1, "Bridge Type", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(edsObject{0x2003, 0x2, "LegacyVersion", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(edsObject{0x2003, 0x3, "DeviceType", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(edsObject{0x2003, 0x4, "DeviceRevision", 0x7, "RAM", 0x5, "ro", false, 0x0});
    list.push_back(edsObject{0x2003, 0x5, "CoreID", 0x7, "RAM", 0xa, "ro", false, 0x0});
    list.push_back(
        edsObject{0x2004, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0xd});
    list.push_back(edsObject{0x2004, 0x0, "System command", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x2004, 0x1, "Blink", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0x2, "Reset", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0x3, "Calibrate", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0x4, "Calibrate Aux", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0x5, "Zero", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0x6, "Calibrate Current", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0x7, "Test Main Encoder", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0x8, "Test Aux Encoder", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0x9, "Save", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(
        edsObject{0x2004, 0xa, "Revert Factory Settings", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0xb, "Clear Errors", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0xc, "Clear Warnings", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(edsObject{0x2004, 0xd, "Can Reinit", 0x7, "RAM", 0x1, "wo", false, 0x0});
    list.push_back(
        edsObject{0x2005, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x9});
    list.push_back(edsObject{0x2005, 0x0, "System status", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2005, 0x1, "Main Encoder Status", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2005, 0x2, "Aux Encoder Status", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2005, 0x3, "Calibration Status", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2005, 0x4, "Bridge Status", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2005, 0x5, "Hardware Status", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2005, 0x6, "Homing Status", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2005, 0x7, "Motion Status", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(
        edsObject{0x2005, 0x8, "Communication Status", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2005, 0x9, "Misc Status", 0x7, "RAM", 0x7, "ro", false, 0x0});
    list.push_back(edsObject{0x2006, 0x0, "Motor settings", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2006, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x14});
    list.push_back(edsObject{0x2006, 0x1, "Pole Pairs", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0x2, "Torque Constant", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0x3, "Inductance", 0x7, "RAM", 0x8, "ro", false, 0x0});
    list.push_back(edsObject{0x2006, 0x4, "Resistance", 0x7, "RAM", 0x8, "ro", false, 0x0});
    list.push_back(edsObject{0x2006, 0x5, "Torque Bandwidth", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0x6, "Name", 0x7, "RAM", 0x9, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2006, 0x7, "Motor Shutdown Temperature", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0x8, "Calibration Mode", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0x9, "Can ID", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0xa, "Can Datarate", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0xb, "Can Timeout", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0xc, "Can Termination", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0xd, "KV", 0x7, "RAM", 0x6, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0xe, "Torque Constant A", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0xf, "Torque Constant B", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0x10, "Torque Constant C", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0x11, "Friction Dynamic", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0x12, "Friction Static", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0x13, "Shunt Resistance", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2006, 0x14, "Thermistor Type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2008, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0xa});
    list.push_back(edsObject{0x2008, 0x0, "Main encoder", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x2008, 0x1, "Type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2008, 0x2, "Calibration Mode", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2008, 0x3, "Mode", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2008, 0x4, "Direction", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2008, 0x5, "Position", 0x7, "RAM", 0x8, "ro", true, 0x0});
    list.push_back(edsObject{0x2008, 0x6, "Velocity", 0x7, "RAM", 0x8, "ro", true, 0x0});
    list.push_back(edsObject{0x2008, 0x7, "Standard Deviation", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2008, 0x8, "Max Error", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2008, 0x9, "Min Error", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2008, 0xa, "Zero Offset", 0x7, "RAM", 0x4, "rw", false, 0x0});
    list.push_back(edsObject{0x200a, 0x0, "Aux encoder", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x200a, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0xa});
    list.push_back(edsObject{0x200a, 0x1, "Type", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x200a, 0x2, "Calibration Mode", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x200a, 0x3, "Mode", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x200a, 0x4, "Direction", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x200a, 0x5, "Position", 0x7, "RAM", 0x8, "ro", true, 0x0});
    list.push_back(edsObject{0x200a, 0x6, "Velocity", 0x7, "RAM", 0x8, "ro", true, 0x0});
    list.push_back(edsObject{0x200a, 0x7, "Standard Deviation", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x200a, 0x8, "Max Error", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x200a, 0x9, "Min Error", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x200a, 0xa, "Zero Offset", 0x7, "RAM", 0x4, "rw", false, 0x0});
    list.push_back(
        edsObject{0x200f, 0x0, "Velocity PID controller", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x200f, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x5});
    list.push_back(edsObject{0x200f, 0x1, "Kp", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x200f, 0x2, "Ki", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x200f, 0x3, "Kd", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x200f, 0x4, "Integral Limit", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x200f, 0x5, "Output Max", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2010, 0x0, "Position PID controller", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2010, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x5});
    list.push_back(edsObject{0x2010, 0x1, "Kp", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x2010, 0x2, "Ki", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x2010, 0x3, "Kd", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x2010, 0x4, "Integral Limit", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2010, 0x5, "Output Max", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(
        edsObject{0x2011, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x3});
    list.push_back(
        edsObject{0x2011, 0x0, "Impedance PD controller", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(edsObject{0x2011, 0x1, "Kp", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x2011, 0x2, "Kd", 0x7, "RAM", 0x8, "rww", true, 0x0});
    list.push_back(edsObject{0x2011, 0x3, "Output Max", 0x7, "RAM", 0x8, "rw", false, 0x0});
    list.push_back(edsObject{0x2012, 0x0, "Temperature", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2012, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x2});
    list.push_back(edsObject{0x2012, 0x1, "Temperature Motor", 0x7, "RAM", 0x8, "ro", true, 0x0});
    list.push_back(edsObject{0x2012, 0x2, "Temperature Driver", 0x7, "RAM", 0x8, "ro", true, 0x0});
    list.push_back(edsObject{0x2013, 0x0, "User GPIO", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x2013, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x2});
    list.push_back(
        edsObject{0x2013, 0x1, "User GPIO Configuration", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x2013, 0x2, "User GPIO State", 0x7, "RAM", 0x6, "ro", false, 0x0});
    list.push_back(edsObject{0x603f, 0x0, "Error Code", 0x7, "RAM", 0x6, "ro", true, 0x0});
    list.push_back(edsObject{0x6040, 0x0, "Controlword", 0x7, "RAM", 0x6, "rww", true, 0x0});
    list.push_back(edsObject{0x6041, 0x0, "Statusword", 0x7, "RAM", 0x6, "ro", true, 0x0});
    list.push_back(edsObject{0x6060, 0x0, "Modes Of Operation", 0x7, "RAM", 0x2, "rww", true, 0x0});
    list.push_back(
        edsObject{0x6061, 0x0, "Modes Of Operation Display", 0x7, "RAM", 0x2, "ro", true, 0x0});
    list.push_back(
        edsObject{0x6062, 0x0, "Position Demand Value", 0x7, "RAM", 0x4, "ro", true, 0x0});
    list.push_back(
        edsObject{0x6064, 0x0, "Position Actual Value", 0x7, "RAM", 0x4, "ro", true, 0x0});
    list.push_back(edsObject{0x6067, 0x0, "Position Window", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(
        edsObject{0x606b, 0x0, "Velocity Demand Value", 0x7, "RAM", 0x4, "ro", true, 0x0});
    list.push_back(
        edsObject{0x606c, 0x0, "Velocity Actual Value", 0x7, "RAM", 0x4, "ro", true, 0x0});
    list.push_back(edsObject{0x606d, 0x0, "Velocity Window", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(edsObject{0x6071, 0x0, "Target Torque", 0x7, "RAM", 0x3, "rww", true, 0x0});
    list.push_back(edsObject{0x6072, 0x0, "Max Torque", 0x7, "RAM", 0x6, "rww", true, 0x0});
    list.push_back(edsObject{0x6073, 0x0, "Max Current", 0x7, "RAM", 0x6, "rww", true, 0x0});
    list.push_back(edsObject{0x6074, 0x0, "Torque Demand Value", 0x7, "RAM", 0x3, "ro", true, 0x0});
    list.push_back(
        edsObject{0x6075, 0x0, "Motor Rated Current", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x6076, 0x0, "Motor Rated Torque", 0x7, "RAM", 0x7, "rw", false, 0x0});
    list.push_back(edsObject{0x6077, 0x0, "Torque Actual Value", 0x7, "RAM", 0x3, "ro", true, 0x0});
    list.push_back(
        edsObject{0x6079, 0x0, "DC Link Circuit Voltage", 0x7, "RAM", 0x7, "ro", true, 0x0});
    list.push_back(edsObject{0x607a, 0x0, "Target Position", 0x7, "RAM", 0x4, "rww", true, 0x0});
    list.push_back(
        edsObject{0x607b, 0x0, "Position Range Limit", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x607b, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x2});
    list.push_back(
        edsObject{0x607b, 0x1, "Min Position Range Limit", 0x7, "RAM", 0x4, "rww", true, 0x0});
    list.push_back(
        edsObject{0x607b, 0x2, "Max Position Range Limit", 0x7, "RAM", 0x4, "rww", true, 0x0});
    list.push_back(
        edsObject{0x607d, 0x0, "Software Position Limit", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x607d, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x2});
    list.push_back(edsObject{0x607d, 0x1, "Min Position Limit", 0x7, "RAM", 0x4, "rww", true, 0x0});
    list.push_back(edsObject{0x607d, 0x2, "Max Position Limit", 0x7, "RAM", 0x4, "rww", true, 0x0});
    list.push_back(edsObject{0x607e, 0x0, "Polarity", 0x7, "RAM", 0x5, "rw", false, 0x0});
    list.push_back(edsObject{0x6080, 0x0, "Max Motor Speed", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(edsObject{0x6081, 0x0, "Profile Velocity", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(
        edsObject{0x6083, 0x0, "Profile Acceleration", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(
        edsObject{0x6084, 0x0, "Profile Deceleration", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(
        edsObject{0x6085, 0x0, "Quick Stop Deceleration", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(edsObject{0x6091, 0x0, "Gear Ratio", 0x9, "RAM", 0x0, "RO", false, 0x0});
    list.push_back(
        edsObject{0x6091, 0x0, "Highest sub-index supported", 0x7, "RAM", 0x5, "ro", false, 0x2});
    list.push_back(edsObject{0x6091, 0x1, "Motor Revolutions", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(edsObject{0x6091, 0x2, "Shaft Revolutions", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(
        edsObject{0x60a8, 0x0, "SI Unit Position", 0x7, "RAM", 0x7, "ro", false, 0xb50000});
    list.push_back(
        edsObject{0x60a9, 0x0, "SI Unit Velocity", 0x7, "RAM", 0x7, "ro", false, 0xfdb44700});
    list.push_back(
        edsObject{0x60aa, 0x0, "SI Unit Acceleration", 0x7, "RAM", 0x7, "ro", false, 0xfdc00300});
    list.push_back(edsObject{0x60c5, 0x0, "Max Acceleration", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(edsObject{0x60c6, 0x0, "Max Deceleration", 0x7, "RAM", 0x7, "rww", true, 0x0});
    list.push_back(edsObject{0x60ff, 0x0, "Target Velocity", 0x7, "RAM", 0x4, "rww", true, 0x0});
    list.push_back(
        edsObject{0x6502, 0x0, "Supported Drive Modes", 0x7, "RAM", 0x7, "ro", false, 0x287});
    return list;
}
