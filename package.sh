#!/bin/bash
##########################################################################################
## keyd: Daemon for tracking hardware keys, written by EXL for/from forum.motofan.ru, 2016
## testers: VINRARUS, fill.sa
## others: Put this config in the same directory with the keyd executable binary or to the
## /ezxlocal/download/appwrite/setup/ directory
## license: Public Domain
## version: v1.0 | 09-FEB-2017
##########################################################################################

# Name
PROJECT="keyd"
VERSION="v1.0_09-FEB-2017"
PACKAGE_NAME=keyd-$VERSION.tar.gz

# Dirs
CURRENT_DIR="`pwd`"
PACKAGE_DIR="$CURRENT_DIR/$PROJECT"

func_package() {
    # Create dirs
    echo -n "Prepare directory $PACKAGE_DIR for package app and building..."
    mkdir -p $PACKAGE_DIR

    # Build executable's
    make PLATFORM=EZX-Z6
    make PLATFORM=EZX-V8
    make PLATFORM=EZX-E8
    make PLATFORM=EZX-EM30
    make PLATFORM=EZX-U9
    make PLATFORM=EZX-Z6W
    make PLATFORM=EZX-ZN5
    make PLATFORM=EZX-EM35
    make PLATFORM=EZX-VE66

    # Copy package files
    cp keyd_* $PACKAGE_DIR
    cp keyd.cfg $PACKAGE_DIR
    cp README.md $PACKAGE_DIR
    echo "done."

    # Archive files
    echo -n "Package files to $PACKAGE_NAME..."
    tar -czf $PACKAGE_NAME $PROJECT/
    echo "done."

    # Clean
    echo -n "Cleaning $PACKAGE_DIR and executables..."
    rm -Rf $PACKAGE_DIR
    make clean
    echo "done."
}

func_package ;
exit 1
