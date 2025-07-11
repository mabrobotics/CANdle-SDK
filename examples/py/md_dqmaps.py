import pyCandle as pc
import numpy as np
import csv
import sys

nVoltage = 5
nRows    = 17
nCols    = 15

maps = np.zeros((2, nVoltage, nRows, nCols), dtype=np.float32)

voltages = np.array([36, 42, 48, 54, 60], dtype=np.float32)
torques = np.array([i for i in range(17)], dtype=np.float32)
velocities = np.array([i * 5 for i in range(15)], dtype=np.float32)

def read_csv(filepath):
    with open(filepath, 'r') as fd:
        lines = list(fd)
        if len(lines) != 1 + 2 * nVoltage * nRows:
            print(f"Unexpected number of lines!\nGot: {len(lines)}\nExpected: {1 + 2 * nVoltage * nRows}")
            return False
        header = lines[0].strip().split(',')
        expected_header = ["idiq", "voltage", "torque"] + [f"rpm{i}" for i in range(nCols)]
        if header != expected_header:
            print(f"CSV header does not match!\nGot: {header}\nExpected: {expected_header}")
            return False

        for dq in range(2):
            for volt in range(nVoltage):
                for row in range(nRows):
                    idx = 1 + (dq * nVoltage * nRows + volt * nRows + row)
                    values = lines[idx].strip().split(',')
                    if len(values) != 3 + nCols:
                        print(f"Incorrect number of columns at line {idx + 1}: {len(values)}")
                        return False
                    try:
                        rpm_values = [float(val) for val in values[3:]]
                    except ValueError as e:
                        print(f"Conversion error at line {idx + 1}: {e}")
                        return False
                    maps[dq][volt][row] = rpm_values

    return True

def main():
    if len(sys.argv) != 2:
        print(f"Usage: python {sys.argv[0]} <path_to_csv>")
        sys.exit(1)

    csv_path = sys.argv[1]
    if read_csv(csv_path):
        print("CSV parsed successfully.")
    else:
        print("Failed to parse CSV.")
        
    candle = pc.attachCandle(pc.CANdleBaudrate_E.CAN_BAUD_1M, pc.busTypes_t.USB)

    # Create virual MD representation
    md = pc.MD(100, candle)

    # Initialize it to see if it connects
    err = md.init()

    print(f"MD initialized with following status: {err}")
    
    if(
        pc.writeRegisterFloatArray(md, "mapVoltageValues", voltages) != 1 or
        pc.writeRegisterFloatArray(md, "mapTorqueValues0", torques[:15]) != 1 or
        pc.writeRegisterFloatArray(md, "mapTorqueValues1", torques[-2:]) != 1 or
        pc.writeRegisterFloatArray(md, "mapVelocityValues", velocities) != 1):
        print("Failed to set up map!")
        sys.exit(1)
    
    print("voltages:", ", ".join(f"{v:.1f}" for v in voltages))
    print("torques:", ", ".join(f"{t:.1f}" for t in torques))
    print("velocities:", ", ".join(f"{v:.1f}" for v in velocities))
    
    for dq in range(2):
        for volt in range(nVoltage):
            for row in range(nRows):
                if pc.writeRegisterU8Array(md, "mapSelectRow", np.array([dq, volt, row], dtype=np.uint8)) != 1:
                    print(f"Failed to select row: dq={dq}, volt={volt}, row={row}", file=sys.stderr)
                    sys.exit(1)

                if pc.writeRegisterFloatArray(md, "mapRowData", maps[dq, volt, row]) != 1:
                    print(f"Failed to write mapRowData: dq={dq}, volt={volt}, row={row}", file=sys.stderr)
                    sys.exit(1)

                readMapRowData, err = pc.readRegisterFloatArray(md, "mapRowData")
                if err != 1:
                    print(f"Failed to read mapRowData for verification: dq={dq}, volt={volt}, row={row}", file=sys.stderr)
                    sys.exit(1)

                if not np.allclose(readMapRowData, maps[dq, volt, row], atol=1e-6):
                    print(
                        f"Verification failed at dq={dq}, volt={volt}, row={row}\n"
                        f"Expected: {maps[dq, volt, row]}\nGot: {readMapRowData}",
                        file=sys.stderr
                    )
                    sys.exit(1)

    resp = input("Enable Maps? [y/n] ").strip().lower()
    if resp != 'y':
        sys.exit(0)

    if (
        pc.writeRegisterU8(md, "iqControlMode", 1) != 1 or
        pc.writeRegisterU8(md, "idControlMode", 1) != 1
    ):
        print("Failed to enable Maps!", file=sys.stderr)
        sys.exit(1) 
    
if __name__=="__main__":
    main()