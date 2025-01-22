#!/bin/bash

set -e
set -x
# Check if the argument 1 is a file
if [ ! -f "$2" ]; then
    echo "Invalid path for the control file"
    # exit 1
fi

# Check if the argument 2 is a file
if [ ! -f "$3" ]; then
    echo "Invalid path of the .bin file"
    exit 1
fi

PACKAGE_NAME=$1

echo SCRIPT IS RUNNING FROM $(pwd)

mkdir -p deb_package/$PACKAGE_NAME/DEBIAN
mkdir -p deb_package/$PACKAGE_NAME/usr/bin
mkdir -p deb_package/$PACKAGE_NAME/etc
cp $2 deb_package/$PACKAGE_NAME/DEBIAN/control
cp $3 deb_package/$PACKAGE_NAME/usr/bin/
cp -r udev deb_package/$PACKAGE_NAME/etc/


dpkg-deb --build deb_package/$PACKAGE_NAME

