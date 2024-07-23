"""
This script generates a pybind module from the register.hpp file
The path can be specified as an argument, if no path is given, the default path is used (./src/candle/include/register.hpp)
The output is written to the file enum_output.txt in the same directory as the script
"""

import sys
import csv

# if no path is given, the default path is used
if(len(sys.argv) == 1):
    path = "../candlelib/include/register.hpp"
else:
    path = sys.argv[1]

sourceFile = open(path, "r")
destination = open("./enum_output.txt", "w+")

destination.write("CHAR ARRAYS ARE NOT INCLUDED BECOUSE SUPOSEDLY THEY DON'T WORK WITH PYBIND\n\n")

"""
read the enum and return a csv array

key: the key string to search for
sourceFile: the file object to search in

return: a list of lists containing the enum members, first element is the name, second is the value
"""
def read_enum(key, sourceFile):
    sourceFile.seek(0)
    begin_enum = False
    csv_buffer = ""
    for line in sourceFile:
        if key in line:
            begin_enum = True
            continue
        if begin_enum:
            if "};" in line:
                break
            elif "{" in line:
                continue
            elif line == "\n":
                csv_buffer += "break=here\n"
            else:
                csv_buffer += line.replace("\n", "").replace(" ", "").replace("\t", "").replace(",", "").replace("\r","") + "\n"
    csv_array = csv.reader(csv_buffer.splitlines(), delimiter='=')
    return csv_array

def read_enum_reversed(key, sourceFile):
    sourceFile.seek(0)
    begin_enum = False
    csv_buffer = ""
    for line in reversed(list(sourceFile)):
        if key in line:
            begin_enum = True
            continue
        if begin_enum:
            if "{" in line:
                break
            elif line == "\n":
                csv_buffer += "break=here\n"
            else:
                csv_buffer += line.replace("\n", "").replace(" ", "").replace("\t", "").replace(",", "").replace("\r","") + "\n"
    #invert the order of the lines
    csv_buffer = csv_buffer.splitlines()
    csv_buffer.reverse()
    csv_buffer = "\n".join(csv_buffer)
    csv_array = csv.reader(csv_buffer.splitlines(), delimiter='=')
    return csv_array

"""
read the struct and return a csv array
the struct is read in reverse order because its easier that way

key: the key string to search for
sourceFile: the file to search in

return: a list of lists containing the struct members, first element is the type, second is the name
"""
def read_struct_reversed(key, sourceFile):
    sourceFile.seek(0)
    begin_struct = False
    csv_buffer = ""
    for line in reversed(list(sourceFile)):
        if key in line:
            begin_struct = True
            continue
        if begin_struct:
            if "{" in line:
                break
            elif "}" in line:
                continue
            elif "char" in line and "[" in line: # char arrays are not included because they don't work with pybind
                continue
            elif line == "\n":
                csv_buffer += "break here\n"    
            else:
                for char in line:
                    if char == " ":
                        continue
                    else:
                        break
                csv_buffer += line.replace("\n", "").replace("\t", "").replace(",", "").replace("\r","").replace(";","") + "\n"
    csv_array = csv.reader(csv_buffer.splitlines(), delimiter=' ')
    csv_array = list(csv_array)
    csv_array.reverse()
    return csv_array

# read the file and write the output
csv_array_all = read_enum_reversed("} Md80Reg_E;", sourceFile)
csv_array_RW = read_struct_reversed("} regRW_st;", sourceFile)
csv_array_RO = read_struct_reversed("} regRO_st;", sourceFile)

sourceFile.close()

# format the data and write it to the output file for the enum
destination.write("py::enum_<mab::Md80Reg_E>(m, \"Md80Reg_E\")\n")
for row in csv_array_all:
    if(row[0] == "break" and row[1] == "here"):
        destination.write("\n")
    else:
        destination.write("    .value(\"" + row[0] + "\", mab::Md80Reg_E::" + row[0] + ")\n")
destination.write("    .export_values();")
destination.write("\n\n")

# format the data and write it to the output file for the read-only struct
destination.write("py::class_<mab::regRO_st>(m, \"regRO_st\")\n    .def(py::init())\n")
for row in csv_array_RO:
    if(row[0] == "break" and row[1] == "here"):
        destination.write("\n")
    else:
        destination.write("    .def_readwrite(\"" + row[1] + "\", &mab::regRO_st::" + row[1] + ")\n")
destination.write(";")
destination.write("\n\n")

# format the data and write it to the output file for the read-write struct
destination.write("py::class_<mab::regRW_st>(m, \"regRW_st\")\n    .def(py::init())\n")
for row in csv_array_RW:
    if(row[0] == "break" and row[1] == "here"):
        destination.write("\n")
    else:
        destination.write("    .def_readwrite(\"" + row[1] + "\", &mab::regRW_st::" + row[1] + ")\n")
destination.write(";")
destination.write("\n\n")
destination.close()