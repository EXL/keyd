#!/bin/bash
##########################################################################################
## keyd: Daemon for tracking hardware keys, written by EXL for/from forum.motofan.ru, 2016
## testers: VINRARUS, fill.sa
## others: Put this config in the same directory with the keyd executable binary or to the
## /ezxlocal/download/appwrite/setup/ directory
## license: Public Domain
## version: v1.0 | 13-FEB-2017
##########################################################################################

# Name
PROJECT="keyd"
VERSION="v1.0_13-FEB-2017"
PACKAGE_NAME=keyd-$VERSION.tar.gz

# Dirs
CURRENT_DIR="`pwd`"
PACKAGE_DIR="$CURRENT_DIR/$PROJECT"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

func_package() {
    # Create dirs
    echo -e "${BLUE}Prepare directory $PACKAGE_DIR for package app and building...${NC}"
    mkdir -p $PACKAGE_DIR

    # Build executable's
    make PLATFORM=EZX-Z6 ; make rmobj
    make PLATFORM=EZX-V8 ; make rmobj
    make PLATFORM=EZX-E8 ; make rmobj
    make PLATFORM=EZX-EM30 ; make rmobj
    make PLATFORM=EZX-U9 ; make rmobj
    make PLATFORM=EZX-Z6W ; make rmobj
    make PLATFORM=EZX-ZN5 ; make rmobj
    make PLATFORM=EZX-EM35 ; make rmobj
    make PLATFORM=EZX-VE66 ; make rmobj

    # Copy package files
    cp keyd_* $PACKAGE_DIR
    cp keyd.cfg $PACKAGE_DIR
    cp README.md $PACKAGE_DIR
    echo -e "${GREEN}Done!${NC}"

    # Archive files
    echo -e "${BLUE}Package files to $PACKAGE_NAME...${NC}"
    tar -czf $PACKAGE_NAME $PROJECT/
    echo -e "${GREEN}Done!${NC}"

    # Clean
    echo -e "${BLUE}Cleaning $PACKAGE_DIR and executables...${NC}"
    rm -Rf $PACKAGE_DIR
    make clean
    echo -e "${GREEN}Done!${NC}"
}

func_package ;
exit 1
