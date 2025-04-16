#!/bin/bash

if test -f ./venv/bin/python3; then
    echo "Python virtual environment already exists."
else
    echo "Creating Python virtual environment..."
    python3 -m venv ./venv
fi

if ./venv/bin/python3 -m build; then
    echo "Built pycandle successfully"
else
    echo -e "\e[31mError in building pycandle\e[0m"
    exit 1
fi

if ./venv/bin/python3 -m pip install --force-reinstall ./dist/pycandlemab-*-linux_x86_64.whl; then
    echo "Installed pycandle successfully"
else
    echo -e "\e[31mError in installing pycandle\e[0m"
    exit 1
fi

