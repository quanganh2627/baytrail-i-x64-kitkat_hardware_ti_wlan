#!/bin/bash

function print_highlight {
echo -e "\033[7m$1\033[0m"
sleep 1
} 


function prepare_wilink_test_folder {
cd ~/
rm -rf WiLink_Test
mkdir  WiLink_Test
cd WiLink_Test
} 

# $1 - holds the packages name to be created (i.e. zoom3Binaries.tar)
#This script should be run from pltaform/os/linux/script directory

#export CROSS_COMPILE=arm-none-linux-gnueabi-
export ARCH=arm
export TI_HOST_OS=ANDROID
export HOST_PLATFORM=zoom2
export PATH=$PATH:/apps/android/arm-2008q3/bin/:/home/a0798970/bin
export ANDR_TI_SUPP_LIB_DIR=../../../../external_suppl

if [ "$KERNEL_DIR" = "" ]; then
	print_highlight "Error, KERNEL_DIR variable has not defined yet. Please define it and run the script again! "
    exit
fi

if [ "$CROSS_COMPILE" = "" ]; then
        print_highlight  "CROSS_COMPILE has not be predefined, The default tool chain will be: arm-none-linux-gnueabi- . "
	export CROSS_COMPILE=arm-none-linux-gnueabi-
fi



WILINK_ROOT="$PWD/../../../../"
OUT_DIR_NAME="$PWD/version_tree/packages"
SOURCES_DIR_NAME="$PWD/version_tree/sources"
WILINK_PACKAGE_PREFIX="mcp_linux_source"


print_highlight "I will now build the wlan $1 packages for you :-) "

rm -rf $OUT_DIR_NAME
mkdir  $OUT_DIR_NAME

####################### Trio Packaging section #############

print_highlight "Building Trio packages... "
mkdir  $OUT_DIR_NAME/trio

# Extract the package in the root directory
prepare_wilink_test_folder
print_highlight "Extracting WiLink NO-CCX tar files (core + osa) ..."
tar -xvjf "$SOURCES_DIR_NAME"/"$WILINK_PACKAGE_PREFIX"_noCCX_core.tar.bz2
tar -xvjf "$SOURCES_DIR_NAME"/"$WILINK_PACKAGE_PREFIX"_noCCX_osa.tar.bz2
print_highlight "Building Trio NoCCX package..."
print_highlight "copying wpa_suppl, scripts & images folders for build success only!!!"
cp -rf $WILINK_ROOT/CUDK/wpa_suppl/ ~/WiLink_Test/WiLink/CUDK/
cp -rf $WILINK_ROOT/platforms/os/linux/scripts/ ~/WiLink_Test/WiLink/platforms/os/linux/
cp -rf $WILINK_ROOT/platforms/os/linux/images/zoom2 ~/WiLink_Test/WiLink/platforms/os/linux/images/
cp -rf $WILINK_ROOT/CUDK/hostapd ~/WiLink_Test/WiLink/CUDK/
cd WiLink/platforms/os/linux
make BMTRACE=n ETH_SUPPORT=y CCX=n SUPPL=WPA BUILD_SUPPL=y TNETW=1273 USE_ANDROID_APPS_ONLY=n
mkdir $OUT_DIR_NAME/trio/NoCCX
cp zoom2Binaries.tar  $OUT_DIR_NAME/trio/NoCCX/softAP_$1


###############################################################


####################### Quattro Packaging section #############

print_highlight "Building Quattro packages..."
mkdir $OUT_DIR_NAME/quattro

# Extract the package in the root directory
prepare_wilink_test_folder
print_highlight "Extracting WiLink NO-CCX tar files (core + osa) ..."
tar -xvjf "$SOURCES_DIR_NAME"/"$WILINK_PACKAGE_PREFIX"_noCCX_core.tar.bz2
tar -xvjf "$SOURCES_DIR_NAME"/"$WILINK_PACKAGE_PREFIX"_noCCX_osa.tar.bz2
print_highlight "Building quattro NoCCX package..."
print_highlight "copying wpa_suppl, scripts & images folders for build success only!!!"
cp -rf $WILINK_ROOT/CUDK/wpa_suppl/ ~/WiLink_Test/WiLink/CUDK/
cp -rf $WILINK_ROOT/platforms/os/linux/scripts/ ~/WiLink_Test/WiLink/platforms/os/linux/
cp -rf $WILINK_ROOT/platforms/os/linux/images/zoom2 ~/WiLink_Test/WiLink/platforms/os/linux/images/
cp -rf $WILINK_ROOT/CUDK/hostapd ~/WiLink_Test/WiLink/CUDK/
cd WiLink/platforms/os/linux
make BMTRACE=n ETH_SUPPORT=y CCX=n SUPPL=WPA BUILD_SUPPL=y TNETW=1283 USE_ANDROID_APPS_ONLY=n
mkdir $OUT_DIR_NAME/quattro/NoCCX
cp zoom2Binaries.tar  $OUT_DIR_NAME/quattro/NoCCX/softAP_$1
###############################################################

print_highlight "Packages were successfully built!" 
