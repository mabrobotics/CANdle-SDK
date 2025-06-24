#include "MD.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>

constexpr u32 nVoltage = 5;
constexpr u32 nRows    = 17;
constexpr u32 nCols    = 15;
f32           maps[2][nVoltage][nRows][nCols];

bool read_csv(char filepath[]);

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Must provide CSV filepath as arg!\n");
        return EXIT_FAILURE;
    }
    if (!read_csv(argv[1]))
    {
        fprintf(stderr, "Parsing CSV failed!\n");
        return EXIT_FAILURE;
    }
    fprintf(stderr, "Parsing CSV successfull!\n");

    auto candle =
        mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, mab::candleTypes::busTypes_t::USB);
    mab::MD md(100, candle);
    if (md.init() != mab::MD::Error_t::OK)
    {
        fprintf(stderr, "Could not initialize MD\n");
        return EXIT_FAILURE;
    }
    mab::MDRegisters_S regs;
    for (u32 dq = 0; dq < 2; dq++)
        for (u32 volt = 0; volt < nVoltage; volt++)
            for (u32 row = 0; row < nRows; row++)
            {
                regs.mapSelectRow.value[0] = dq;
                regs.mapSelectRow.value[1] = volt;
                regs.mapSelectRow.value[2] = row;
                memcpy(&regs.mapRowData.value, maps[dq][volt][row], sizeof(f32) * nCols);
                bool selectOk = md.writeRegisters(regs.mapSelectRow) == mab::MD::Error_t::OK;
                bool dataOk   = md.writeRegisters(regs.mapRowData) == mab::MD::Error_t::OK;
                memset(regs.mapRowData.value, 0, 60);
                bool readOk   = md.readRegister(regs.mapRowData) == mab::MD::Error_t::OK;
                if (readOk)
                    for (u32 i = 0; i < nCols; i++)
                    {
                        f32 ptr = (regs.mapRowData.value)[i];
                        readOk &= maps[dq][volt][row][i] == ptr;
                    }
                fprintf(stderr,
                        "%s %d %d = select: %s, data: %s, verify: %s\n",
                        dq == 0 ? "ID" : "ID",
                        volt,
                        row,
                        selectOk ? "OK" : "NOK",
                        dataOk ? "OK" : "NOK",
                        readOk ? "OK" : "NOK");
            }
}

bool read_csv(char filepath[])
{
    FILE* fd = fopen(filepath, "r");
    if (fd == nullptr)
    {
        fprintf(stderr, "Could not open CSV.\n");
        return false;
    }
    char buf[256];
    fgets(buf, 256, fd);
    if (strcmp(buf,
               "idiq,voltage,torque,rpm0,rpm1,rpm2,rpm3,rpm4,rpm5,rpm6,rpm7,rpm8,rpm9,rpm10,rpm11,"
               "rpm12,rpm13,rpm14\n") != 0)
    {
        fprintf(stderr, "CSV header does not match!\n");
        return false;
    }
    constexpr int csvColumns = nCols + 3;
    for (u32 dq = 0; dq < 2; dq++)
        for (u32 volt = 0; volt < nVoltage; volt++)
            for (u32 row = 0; row < nRows; row++)
            {
                if (fgets(buf, 255, fd) == nullptr)
                {
                    fprintf(stderr, "CSV corrupted here: %d, %d, %d\n", dq, volt, row);
                    return false;
                }
                char* delimiter = buf;
                for (int i = 3; i < csvColumns; i++)
                {
                    char* substring            = delimiter;
                    delimiter                  = strchr(delimiter, ',');
                    *delimiter                 = 0;
                    maps[dq][volt][row][i - 3] = atof(substring);
                    delimiter++;
                    fprintf(stderr, "%.2f,", maps[dq][volt][row][i - 3]);
                }
                fprintf(stderr, "\n");
            }

    return true;
}
