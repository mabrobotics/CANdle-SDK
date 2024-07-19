#!/bin/bash

NAME=${PWD##*/}  # retain the part after the last slash

RED="\e[31m"
ENDCOLOR="\e[0m"

if [ "$NAME" != "CANdle-SDK" ]; then
    echo "SCRIPT NEEDS TO BE RUN FROM CANDLE-SDK'S ROOT DIRECTORY"
fi

# if pipx run build ; then
#     echo "Built pycandle successfuly"
# else
#     echo -e "${RED}Error in building pycandle${ENDCOLOR}"
#     exit 1
# fi


# cibuildwheel will pre-compile against many python APIs and libc distributions
if pipx run cibuildwheel --platform linux --archs x86_64 ; then
    echo "Cross-compiled pycandle successfuly"
else
    echo -e "${RED}Error in cross-building pycandle${ENDCOLOR}"
    exit 1
fi